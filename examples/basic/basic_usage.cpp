#include "pgv_faiss.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

// TODO: Add examples for different index types (HNSW, Flat)
// TODO: Add error handling demonstrations
// TODO: Add configuration validation examples
// TODO: Add batch operations and performance comparisons
// TODO: Add GPU vs CPU performance comparisons
// TODO: Add memory usage monitoring and optimization examples

void generate_random_vectors(std::vector<float>& vectors, std::vector<int64_t>& ids, 
                            int count, int dimension) {
    std::random_device rd;
    std::mt19937 gen(rd());
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

int main() {
    std::cout << "=== PGVector + FAISS SDK Basic Example ===" << std::endl;
    
    pgv_faiss_config_t config = {0};
    config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
    config.dimension = 128;
    config.use_gpu = 0;
    config.gpu_device_id = 0;
    config.index_type = const_cast<char*>("IVFFlat");
    config.nprobe = 10;
    
    pgv_faiss_index_t* index = nullptr;
    
    std::cout << "Initializing PGV-FAISS index..." << std::endl;
    int result = pgv_faiss_init(&config, &index);
    if (result != 0) {
        std::cerr << "Failed to initialize index: " << result << std::endl;
        return 1;
    }
    std::cout << "✓ Index initialized successfully" << std::endl;
    
    const int num_vectors = 10000;
    const int dimension = 128;
    
    std::vector<float> vectors;
    std::vector<int64_t> ids;
    
    std::cout << "Generating " << num_vectors << " random vectors..." << std::endl;
    generate_random_vectors(vectors, ids, num_vectors, dimension);
    std::cout << "✓ Vectors generated" << std::endl;
    
    std::cout << "Adding vectors to index..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    result = pgv_faiss_add_vectors(index, vectors.data(), ids.data(), num_vectors);
    if (result != 0) {
        std::cerr << "Failed to add vectors: " << result << std::endl;
        pgv_faiss_destroy(index);
        return 1;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "✓ Added " << num_vectors << " vectors in " << duration_ms.count() << "ms" << std::endl;
    
    std::vector<float> query_vector(dimension);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    for (int i = 0; i < dimension; ++i) {
        query_vector[i] = dis(gen);
    }
    
    std::cout << "Performing similarity search..." << std::endl;
    const size_t k = 10;
    pgv_faiss_result_t search_result = {0};
    
    start = std::chrono::high_resolution_clock::now();
    result = pgv_faiss_search(index, query_vector.data(), k, &search_result);
    end = std::chrono::high_resolution_clock::now();
    
    if (result != 0) {
        std::cerr << "Search failed: " << result << std::endl;
        pgv_faiss_destroy(index);
        return 1;
    }
    
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "✓ Search completed in " << duration_us.count() << "μs" << std::endl;
    
    std::cout << "\nTop " << search_result.count << " similar vectors:" << std::endl;
    for (size_t i = 0; i < search_result.count; ++i) {
        std::cout << "  ID: " << search_result.ids[i] 
                  << ", Distance: " << search_result.distances[i] << std::endl;
    }
    
    std::cout << "\nSaving index to database..." << std::endl;
    result = pgv_faiss_save_to_db(index, "test_index");
    if (result == 0) {
        std::cout << "✓ Index saved successfully" << std::endl;
    } else {
        std::cout << "⚠ Index save failed (this is expected if DB is not available)" << std::endl;
    }
    
    // TODO: Add examples for:
    // - Loading index from database
    // - Index statistics and information
    // - Vector update and deletion operations
    // - Different distance metrics and search parameters
    // - Batch search with multiple queries
    // - Error recovery and connection retry scenarios
    
    pgv_faiss_free_result(&search_result);
    pgv_faiss_destroy(index);
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
}