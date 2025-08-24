#include "pgv_faiss.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>

struct BenchmarkResult {
    std::string test_name;
    double add_time_ms;
    double search_time_us;
    double memory_usage_mb;
    size_t index_size;
    double recall;
};

class BenchmarkSuite {
private:
    std::vector<BenchmarkResult> results_;
    std::string output_file_;
    
public:
    BenchmarkSuite(const std::string& output_file = "benchmark_results.csv") 
        : output_file_(output_file) {}
    
    void add_result(const BenchmarkResult& result) {
        results_.push_back(result);
    }
    
    void print_results() const {
        std::cout << "\n=== Benchmark Results ===" << std::endl;
        std::cout << std::setw(20) << "Test Name" 
                  << std::setw(12) << "Add Time(ms)" 
                  << std::setw(15) << "Search Time(μs)" 
                  << std::setw(15) << "Memory(MB)" 
                  << std::setw(12) << "Index Size"
                  << std::setw(10) << "Recall" << std::endl;
        std::cout << std::string(88, '-') << std::endl;
        
        for (const auto& result : results_) {
            std::cout << std::setw(20) << result.test_name
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.add_time_ms
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.search_time_us
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.memory_usage_mb
                      << std::setw(12) << result.index_size
                      << std::setw(10) << std::fixed << std::setprecision(3) << result.recall << std::endl;
        }
    }
    
    void save_to_csv() const {
        std::ofstream file(output_file_);
        if (!file.is_open()) {
            std::cerr << "Failed to open output file: " << output_file_ << std::endl;
            return;
        }
        
        file << "Test Name,Add Time (ms),Search Time (μs),Memory (MB),Index Size,Recall\n";
        for (const auto& result : results_) {
            file << result.test_name << ","
                 << result.add_time_ms << ","
                 << result.search_time_us << ","
                 << result.memory_usage_mb << ","
                 << result.index_size << ","
                 << result.recall << "\n";
        }
        
        std::cout << "Results saved to: " << output_file_ << std::endl;
    }
};

class DatasetGenerator {
public:
    static void generate_random_dataset(std::vector<float>& vectors, std::vector<int64_t>& ids,
                                       int count, int dimension, int seed = 42) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
        
        vectors.resize(count * dimension);
        ids.resize(count);
        
        for (int i = 0; i < count; ++i) {
            ids[i] = i;
            for (int j = 0; j < dimension; ++j) {
                vectors[i * dimension + j] = dis(gen);
            }
        }
    }
    
    static void generate_gaussian_clusters(std::vector<float>& vectors, std::vector<int64_t>& ids,
                                         int num_clusters, int points_per_cluster, int dimension, int seed = 42) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution<float> center_dis(-10.0f, 10.0f);
        std::normal_distribution<float> point_dis(0.0f, 2.0f);
        
        int total_points = num_clusters * points_per_cluster;
        vectors.resize(total_points * dimension);
        ids.resize(total_points);
        
        for (int cluster = 0; cluster < num_clusters; ++cluster) {
            std::vector<float> center(dimension);
            for (int d = 0; d < dimension; ++d) {
                center[d] = center_dis(gen);
            }
            
            for (int point = 0; point < points_per_cluster; ++point) {
                int idx = cluster * points_per_cluster + point;
                ids[idx] = idx;
                
                for (int d = 0; d < dimension; ++d) {
                    vectors[idx * dimension + d] = center[d] + point_dis(gen);
                }
            }
        }
    }
};

double calculate_recall(const std::vector<int64_t>& ground_truth, const pgv_faiss_result_t& result) {
    if (result.count == 0 || ground_truth.empty()) return 0.0;
    
    int matches = 0;
    size_t check_count = std::min(ground_truth.size(), result.count);
    
    for (size_t i = 0; i < check_count; ++i) {
        for (size_t j = 0; j < result.count; ++j) {
            if (ground_truth[i] == result.ids[j]) {
                matches++;
                break;
            }
        }
    }
    
    return double(matches) / check_count;
}

void benchmark_index_type(BenchmarkSuite& suite, const std::string& index_type, 
                         const std::vector<float>& vectors, const std::vector<int64_t>& ids,
                         int dimension, const std::vector<float>& queries, int num_queries) {
    
    std::cout << "Benchmarking " << index_type << " index..." << std::endl;
    
    pgv_faiss_config_t config = {0};
    config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
    config.dimension = dimension;
    config.use_gpu = 0;
    config.index_type = const_cast<char*>(index_type.c_str());
    config.nprobe = 10;
    
    pgv_faiss_index_t* index = nullptr;
    if (pgv_faiss_init(&config, &index) != 0) {
        std::cerr << "Failed to initialize " << index_type << " index" << std::endl;
        return;
    }
    
    auto add_start = std::chrono::high_resolution_clock::now();
    int add_result = pgv_faiss_add_vectors(index, vectors.data(), ids.data(), vectors.size() / dimension);
    auto add_end = std::chrono::high_resolution_clock::now();
    
    if (add_result != 0) {
        std::cerr << "Failed to add vectors to " << index_type << " index" << std::endl;
        pgv_faiss_destroy(index);
        return;
    }
    
    auto add_duration = std::chrono::duration_cast<std::chrono::milliseconds>(add_end - add_start);
    
    std::vector<double> search_times;
    double total_recall = 0.0;
    
    for (int q = 0; q < num_queries; ++q) {
        const float* query = queries.data() + q * dimension;
        
        auto search_start = std::chrono::high_resolution_clock::now();
        pgv_faiss_result_t result = {0};
        int search_result = pgv_faiss_search(index, query, 10, &result);
        auto search_end = std::chrono::high_resolution_clock::now();
        
        if (search_result == 0) {
            auto search_duration = std::chrono::duration_cast<std::chrono::microseconds>(search_end - search_start);
            search_times.push_back(search_duration.count());
            
            pgv_faiss_free_result(&result);
        }
    }
    
    double avg_search_time = 0.0;
    if (!search_times.empty()) {
        avg_search_time = std::accumulate(search_times.begin(), search_times.end(), 0.0) / search_times.size();
    }
    
    BenchmarkResult result;
    result.test_name = index_type;
    result.add_time_ms = add_duration.count();
    result.search_time_us = avg_search_time;
    result.memory_usage_mb = 0.0; 
    result.index_size = vectors.size() / dimension;
    result.recall = total_recall / num_queries;
    
    suite.add_result(result);
    pgv_faiss_destroy(index);
}

void scalability_benchmark(BenchmarkSuite& suite) {
    std::cout << "\n=== Scalability Benchmark ===" << std::endl;
    
    const int dimension = 256;
    const std::vector<int> dataset_sizes = {1000, 5000, 10000, 50000, 100000};
    
    for (int size : dataset_sizes) {
        std::cout << "Testing with " << size << " vectors..." << std::endl;
        
        std::vector<float> vectors;
        std::vector<int64_t> ids;
        DatasetGenerator::generate_random_dataset(vectors, ids, size, dimension);
        
        std::vector<float> queries;
        std::vector<int64_t> query_ids;
        DatasetGenerator::generate_random_dataset(queries, query_ids, 100, dimension, 123);
        
        benchmark_index_type(suite, "IVFFlat_" + std::to_string(size), vectors, ids, 
                            dimension, queries, 100);
    }
}

void dimensionality_benchmark(BenchmarkSuite& suite) {
    std::cout << "\n=== Dimensionality Benchmark ===" << std::endl;
    
    const int num_vectors = 10000;
    const std::vector<int> dimensions = {64, 128, 256, 512, 1024};
    
    for (int dim : dimensions) {
        std::cout << "Testing with " << dim << " dimensions..." << std::endl;
        
        std::vector<float> vectors;
        std::vector<int64_t> ids;
        DatasetGenerator::generate_random_dataset(vectors, ids, num_vectors, dim);
        
        std::vector<float> queries;
        std::vector<int64_t> query_ids;
        DatasetGenerator::generate_random_dataset(queries, query_ids, 100, dim, 123);
        
        benchmark_index_type(suite, "Dim_" + std::to_string(dim), vectors, ids, 
                            dim, queries, 100);
    }
}

void index_comparison_benchmark(BenchmarkSuite& suite) {
    std::cout << "\n=== Index Type Comparison ===" << std::endl;
    
    const int dimension = 256;
    const int num_vectors = 20000;
    
    std::vector<float> vectors;
    std::vector<int64_t> ids;
    DatasetGenerator::generate_gaussian_clusters(vectors, ids, 50, 400, dimension);
    
    std::vector<float> queries;
    std::vector<int64_t> query_ids;
    DatasetGenerator::generate_random_dataset(queries, query_ids, 200, dimension, 456);
    
    std::vector<std::string> index_types = {"Flat", "IVFFlat", "HNSW"};
    
    for (const auto& index_type : index_types) {
        benchmark_index_type(suite, index_type + "_comp", vectors, ids, 
                            dimension, queries, 200);
    }
}

int main() {
    std::cout << "=== PGVector + FAISS Benchmark Suite ===" << std::endl;
    
    BenchmarkSuite suite("pgv_faiss_benchmark_results.csv");
    
    scalability_benchmark(suite);
    
    dimensionality_benchmark(suite);
    
    index_comparison_benchmark(suite);
    
    suite.print_results();
    suite.save_to_csv();
    
    std::cout << "\n=== Benchmark suite completed ===" << std::endl;
    return 0;
}