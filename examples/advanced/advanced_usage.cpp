#include "pgv_faiss.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>

class VectorDataGenerator {
public:
    static void generate_clustered_data(std::vector<float>& vectors, std::vector<int64_t>& ids,
                                      int num_clusters, int vectors_per_cluster, int dimension) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> center_dis(-10.0f, 10.0f);
        std::normal_distribution<float> cluster_dis(0.0f, 1.0f);
        
        int total_vectors = num_clusters * vectors_per_cluster;
        vectors.resize(total_vectors * dimension);
        ids.resize(total_vectors);
        
        for (int cluster = 0; cluster < num_clusters; ++cluster) {
            std::vector<float> center(dimension);
            for (int d = 0; d < dimension; ++d) {
                center[d] = center_dis(gen);
            }
            
            for (int v = 0; v < vectors_per_cluster; ++v) {
                int idx = cluster * vectors_per_cluster + v;
                ids[idx] = idx;
                
                for (int d = 0; d < dimension; ++d) {
                    vectors[idx * dimension + d] = center[d] + cluster_dis(gen);
                }
            }
        }
    }
    
    static std::vector<float> create_query_vector(const std::vector<float>& vectors, 
                                                 int dimension, int base_idx) {
        std::vector<float> query(dimension);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<float> noise(0.0f, 0.1f);
        
        for (int d = 0; d < dimension; ++d) {
            query[d] = vectors[base_idx * dimension + d] + noise(gen);
        }
        
        return query;
    }
};

class PerformanceProfiler {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string operation_name;
    
public:
    PerformanceProfiler(const std::string& name) : operation_name(name) {
        start_time = std::chrono::high_resolution_clock::now();
        std::cout << "[PROFILER] Starting: " << operation_name << std::endl;
    }
    
    ~PerformanceProfiler() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "[PROFILER] Completed: " << operation_name 
                  << " in " << duration.count() << "ms" << std::endl;
    }
};

void test_different_index_types(const std::vector<float>& vectors, const std::vector<int64_t>& ids,
                               int dimension, const std::vector<float>& query) {
    std::vector<std::string> index_types = {"Flat", "IVFFlat", "HNSW"};
    
    for (const auto& index_type : index_types) {
        std::cout << "\n=== Testing " << index_type << " Index ===" << std::endl;
        
        pgv_faiss_config_t config = {0};
        config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
        config.dimension = dimension;
        config.use_gpu = 0;
        config.index_type = const_cast<char*>(index_type.c_str());
        config.nprobe = 10;
        
        pgv_faiss_index_t* index = nullptr;
        if (pgv_faiss_init(&config, &index) != 0) {
            std::cerr << "Failed to initialize " << index_type << " index" << std::endl;
            continue;
        }
        
        {
            PerformanceProfiler profiler(index_type + " vector addition");
            if (pgv_faiss_add_vectors(index, vectors.data(), ids.data(), vectors.size() / dimension) != 0) {
                std::cerr << "Failed to add vectors to " << index_type << " index" << std::endl;
                pgv_faiss_destroy(index);
                continue;
            }
        }
        
        const int num_queries = 100;
        auto query_start = std::chrono::high_resolution_clock::now();
        
        for (int q = 0; q < num_queries; ++q) {
            pgv_faiss_result_t result = {0};
            if (pgv_faiss_search(index, query.data(), 10, &result) == 0) {
                pgv_faiss_free_result(&result);
            }
        }
        
        auto query_end = std::chrono::high_resolution_clock::now();
        auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(query_end - query_start);
        
        std::cout << "Average query time: " << query_duration.count() / num_queries << "μs" << std::endl;
        
        pgv_faiss_destroy(index);
    }
}

void batch_processing_demo(int dimension) {
    std::cout << "\n=== Batch Processing Demo ===" << std::endl;
    
    const int batch_size = 1000;
    const int num_batches = 10;
    
    pgv_faiss_config_t config = {0};
    config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
    config.dimension = dimension;
    config.use_gpu = 0;
    config.index_type = const_cast<char*>("IVFFlat");
    config.nprobe = 10;
    
    pgv_faiss_index_t* index = nullptr;
    if (pgv_faiss_init(&config, &index) != 0) {
        std::cerr << "Failed to initialize index for batch demo" << std::endl;
        return;
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    for (int batch = 0; batch < num_batches; ++batch) {
        std::vector<float> batch_vectors(batch_size * dimension);
        std::vector<int64_t> batch_ids(batch_size);
        
        for (int i = 0; i < batch_size; ++i) {
            batch_ids[i] = batch * batch_size + i;
            for (int d = 0; d < dimension; ++d) {
                batch_vectors[i * dimension + d] = dis(gen);
            }
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        int result = pgv_faiss_add_vectors(index, batch_vectors.data(), batch_ids.data(), batch_size);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        if (result == 0) {
            std::cout << "Batch " << (batch + 1) << "/" << num_batches 
                      << " processed in " << duration.count() << "ms" << std::endl;
        } else {
            std::cerr << "Failed to process batch " << (batch + 1) << std::endl;
        }
    }
    
    pgv_faiss_destroy(index);
}

void memory_usage_demo() {
    std::cout << "\n=== Memory Usage Patterns ===" << std::endl;
    
    const int dimension = 256;
    const std::vector<int> dataset_sizes = {1000, 5000, 10000, 50000};
    
    for (int size : dataset_sizes) {
        std::cout << "\nTesting with " << size << " vectors..." << std::endl;
        
        std::vector<float> vectors;
        std::vector<int64_t> ids;
        VectorDataGenerator::generate_clustered_data(vectors, ids, 10, size / 10, dimension);
        
        pgv_faiss_config_t config = {0};
        config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
        config.dimension = dimension;
        config.use_gpu = 0;
        config.index_type = const_cast<char*>("IVFFlat");
        
        pgv_faiss_index_t* index = nullptr;
        if (pgv_faiss_init(&config, &index) == 0) {
            {
                PerformanceProfiler profiler("Add " + std::to_string(size) + " vectors");
                pgv_faiss_add_vectors(index, vectors.data(), ids.data(), size);
            }
            
            std::vector<float> query = VectorDataGenerator::create_query_vector(vectors, dimension, 0);
            
            pgv_faiss_result_t result = {0};
            auto search_start = std::chrono::high_resolution_clock::now();
            pgv_faiss_search(index, query.data(), 20, &result);
            auto search_end = std::chrono::high_resolution_clock::now();
            
            auto search_duration = std::chrono::duration_cast<std::chrono::microseconds>(search_end - search_start);
            std::cout << "Search time: " << search_duration.count() << "μs" << std::endl;
            
            pgv_faiss_free_result(&result);
            pgv_faiss_destroy(index);
        }
    }
}

int main() {
    std::cout << "=== Advanced PGVector + FAISS SDK Example ===" << std::endl;
    
    const int dimension = 128;
    const int num_clusters = 20;
    const int vectors_per_cluster = 500;
    
    std::vector<float> vectors;
    std::vector<int64_t> ids;
    
    std::cout << "Generating clustered dataset..." << std::endl;
    VectorDataGenerator::generate_clustered_data(vectors, ids, num_clusters, vectors_per_cluster, dimension);
    std::cout << "✓ Generated " << vectors.size() / dimension << " vectors in " << num_clusters << " clusters" << std::endl;
    
    std::vector<float> query = VectorDataGenerator::create_query_vector(vectors, dimension, 42);
    
    test_different_index_types(vectors, ids, dimension, query);
    
    batch_processing_demo(dimension);
    
    memory_usage_demo();
    
    std::cout << "\n=== Advanced example completed ===" << std::endl;
    return 0;
}