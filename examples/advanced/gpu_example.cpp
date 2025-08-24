#include "pgv_faiss.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#ifdef WITH_GPU
#include <cuda_runtime.h>

void print_gpu_info() {
    int device_count;
    cudaGetDeviceCount(&device_count);
    
    std::cout << "=== GPU Information ===" << std::endl;
    std::cout << "Found " << device_count << " CUDA devices:" << std::endl;
    
    for (int i = 0; i < device_count; ++i) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, i);
        
        std::cout << "Device " << i << ": " << prop.name << std::endl;
        std::cout << "  Compute Capability: " << prop.major << "." << prop.minor << std::endl;
        std::cout << "  Total Memory: " << prop.totalGlobalMem / (1024*1024) << " MB" << std::endl;
        std::cout << "  Multiprocessors: " << prop.multiProcessorCount << std::endl;
        std::cout << "  Max Threads per Block: " << prop.maxThreadsPerBlock << std::endl;
        std::cout << "  Memory Clock Rate: " << prop.memoryClockRate / 1000 << " MHz" << std::endl;
        std::cout << "  Memory Bus Width: " << prop.memoryBusWidth << " bits" << std::endl;
        
        size_t free_mem, total_mem;
        cudaSetDevice(i);
        cudaMemGetInfo(&free_mem, &total_mem);
        std::cout << "  Available Memory: " << free_mem / (1024*1024) << " MB" << std::endl;
        std::cout << std::endl;
    }
}

void benchmark_cpu_vs_gpu(int dimension, int num_vectors) {
    std::cout << "=== CPU vs GPU Performance Comparison ===" << std::endl;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    std::vector<float> vectors(num_vectors * dimension);
    std::vector<int64_t> ids(num_vectors);
    
    for (int i = 0; i < num_vectors; ++i) {
        ids[i] = i;
        for (int j = 0; j < dimension; ++j) {
            vectors[i * dimension + j] = dis(gen);
        }
    }
    
    std::vector<float> query(dimension);
    for (int i = 0; i < dimension; ++i) {
        query[i] = dis(gen);
    }
    
    std::cout << "Dataset: " << num_vectors << " vectors, " << dimension << " dimensions" << std::endl;
    
    std::cout << "\n--- CPU Performance ---" << std::endl;
    {
        pgv_faiss_config_t config = {0};
        config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
        config.dimension = dimension;
        config.use_gpu = 0;
        config.index_type = const_cast<char*>("IVFFlat");
        config.nprobe = 10;
        
        pgv_faiss_index_t* cpu_index = nullptr;
        if (pgv_faiss_init(&config, &cpu_index) == 0) {
            auto start = std::chrono::high_resolution_clock::now();
            pgv_faiss_add_vectors(cpu_index, vectors.data(), ids.data(), num_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto add_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "CPU Add time: " << add_time.count() << "ms" << std::endl;
            
            const int num_queries = 1000;
            start = std::chrono::high_resolution_clock::now();
            
            for (int q = 0; q < num_queries; ++q) {
                pgv_faiss_result_t result = {0};
                if (pgv_faiss_search(cpu_index, query.data(), 10, &result) == 0) {
                    pgv_faiss_free_result(&result);
                }
            }
            
            end = std::chrono::high_resolution_clock::now();
            auto search_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "CPU Average query time: " << search_time.count() / num_queries << "μs" << std::endl;
            
            pgv_faiss_destroy(cpu_index);
        }
    }
    
    std::cout << "\n--- GPU Performance ---" << std::endl;
    {
        pgv_faiss_config_t config = {0};
        config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
        config.dimension = dimension;
        config.use_gpu = 1;
        config.gpu_device_id = 0;
        config.index_type = const_cast<char*>("IVFFlat");
        config.nprobe = 10;
        
        pgv_faiss_index_t* gpu_index = nullptr;
        if (pgv_faiss_init(&config, &gpu_index) == 0) {
            auto start = std::chrono::high_resolution_clock::now();
            pgv_faiss_add_vectors(gpu_index, vectors.data(), ids.data(), num_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto add_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "GPU Add time: " << add_time.count() << "ms" << std::endl;
            
            const int num_queries = 1000;
            start = std::chrono::high_resolution_clock::now();
            
            for (int q = 0; q < num_queries; ++q) {
                pgv_faiss_result_t result = {0};
                if (pgv_faiss_search(gpu_index, query.data(), 10, &result) == 0) {
                    pgv_faiss_free_result(&result);
                }
            }
            
            end = std::chrono::high_resolution_clock::now();
            auto search_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "GPU Average query time: " << search_time.count() / num_queries << "μs" << std::endl;
            
            pgv_faiss_destroy(gpu_index);
        } else {
            std::cout << "Failed to initialize GPU index (GPU may not be available)" << std::endl;
        }
    }
}

void test_multiple_gpus() {
    std::cout << "\n=== Multi-GPU Testing ===" << std::endl;
    
    int device_count;
    cudaGetDeviceCount(&device_count);
    
    if (device_count < 2) {
        std::cout << "Only " << device_count << " GPU(s) available. Skipping multi-GPU test." << std::endl;
        return;
    }
    
    const int dimension = 256;
    const int num_vectors = 10000;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    std::vector<float> vectors(num_vectors * dimension);
    std::vector<int64_t> ids(num_vectors);
    
    for (int i = 0; i < num_vectors; ++i) {
        ids[i] = i;
        for (int j = 0; j < dimension; ++j) {
            vectors[i * dimension + j] = dis(gen);
        }
    }
    
    for (int gpu = 0; gpu < std::min(device_count, 4); ++gpu) {
        std::cout << "\nTesting GPU " << gpu << ":" << std::endl;
        
        pgv_faiss_config_t config = {0};
        config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
        config.dimension = dimension;
        config.use_gpu = 1;
        config.gpu_device_id = gpu;
        config.index_type = const_cast<char*>("IVFFlat");
        config.nprobe = 10;
        
        pgv_faiss_index_t* index = nullptr;
        if (pgv_faiss_init(&config, &index) == 0) {
            auto start = std::chrono::high_resolution_clock::now();
            int result = pgv_faiss_add_vectors(index, vectors.data(), ids.data(), num_vectors);
            auto end = std::chrono::high_resolution_clock::now();
            
            if (result == 0) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "  Successfully added vectors in " << duration.count() << "ms" << std::endl;
            } else {
                std::cout << "  Failed to add vectors" << std::endl;
            }
            
            pgv_faiss_destroy(index);
        } else {
            std::cout << "  Failed to initialize index on GPU " << gpu << std::endl;
        }
    }
}

#endif

int main() {
    std::cout << "=== GPU-Accelerated Vector Search Example ===" << std::endl;
    
#ifdef WITH_GPU
    print_gpu_info();
    
    int device_count;
    cudaGetDeviceCount(&device_count);
    
    if (device_count == 0) {
        std::cout << "No CUDA devices found. This example requires a CUDA-capable GPU." << std::endl;
        return 1;
    }
    
    benchmark_cpu_vs_gpu(512, 50000);
    
    test_multiple_gpus();
    
    std::cout << "\n=== GPU example completed ===" << std::endl;
    
#else
    std::cout << "This example was compiled without GPU support." << std::endl;
    std::cout << "Please recompile with -DWITH_GPU=ON to enable GPU features." << std::endl;
    return 1;
#endif
    
    return 0;
}