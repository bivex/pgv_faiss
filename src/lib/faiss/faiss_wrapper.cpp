#include "faiss_wrapper.h"
#include <faiss/IndexFlat.h>
#include <faiss/IndexIVFFlat.h>
#include <faiss/IndexHNSW.h>
#include <faiss/index_io.h>
#include <faiss/AutoTune.h>
#include <iostream>
#include <sstream>

#ifdef WITH_GPU
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/gpu/GpuIndexIVFFlat.h>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/utils/DeviceUtils.h>
#endif

FAISSWrapper::FAISSWrapper(int dimension, const std::string& index_type, 
                           bool use_gpu, int gpu_device)
    : dimension_(dimension), use_gpu_(use_gpu), gpu_device_(gpu_device), 
      index_type_(index_type), trained_(false) {
    
#ifdef WITH_GPU
    if (use_gpu_) {
        setup_gpu_resources();
    }
#else
    if (use_gpu_) {
        std::cerr << "Warning: GPU support not compiled. Using CPU instead." << std::endl;
        use_gpu_ = false;
    }
#endif
    
    index_.reset(create_index(index_type_, dimension_));
}

FAISSWrapper::~FAISSWrapper() = default;

faiss::Index* FAISSWrapper::create_index(const std::string& index_type, int dimension) {
    if (index_type == "Flat") {
#ifdef WITH_GPU
        if (use_gpu_) {
            return new faiss::gpu::GpuIndexFlat(&gpu_resources_, dimension, faiss::METRIC_L2);
        }
#endif
        return new faiss::IndexFlatL2(dimension);
    }
    else if (index_type == "IVFFlat") {
        // TODO: Make ncentroids adaptive based on dataset size
        // TODO: Add support for different distance metrics (cosine, inner product)
        int ncentroids = std::min(4 * (int)sqrt(100000), 65536);
        auto quantizer = new faiss::IndexFlatL2(dimension);
        
#ifdef WITH_GPU
        if (use_gpu_) {
            faiss::gpu::GpuIndexIVFFlat* gpu_index = 
                new faiss::gpu::GpuIndexIVFFlat(&gpu_resources_, dimension, ncentroids, faiss::METRIC_L2);
            return gpu_index;
        }
#endif
        return new faiss::IndexIVFFlat(quantizer, dimension, ncentroids);
    }
    else if (index_type == "HNSW") {
        // TODO: Make M and efConstruction configurable parameters
        // TODO: Add support for different distance metrics
        int M = 16;
        auto index = new faiss::IndexHNSWFlat(dimension, M);
        index->hnsw.efConstruction = 40;
        return index;
    }
    // TODO: Add support for more index types:
    // - IndexIVFPQ for memory-efficient vector quantization
    // - IndexLSH for locality-sensitive hashing
    // - IndexPQ for product quantization
    // - IndexScalarQuantizer for scalar quantization
    
    return new faiss::IndexFlatL2(dimension);
}

#ifdef WITH_GPU
void FAISSWrapper::setup_gpu_resources() {
    if (faiss::gpu::getNumDevices() <= gpu_device_) {
        std::cerr << "GPU device " << gpu_device_ << " not available" << std::endl;
        use_gpu_ = false;
        return;
    }
    
    // TODO: Add configurable GPU memory limits and temp memory settings
    // TODO: Add support for multi-GPU resource management
    // TODO: Implement GPU memory profiling and optimization
    gpu_resources_ = std::make_unique<faiss::gpu::StandardGpuResources>();
}
#else
void FAISSWrapper::setup_gpu_resources() {
    // No-op for CPU-only builds
}
#endif

int FAISSWrapper::add_vectors(const float* vectors, const int64_t* ids, size_t count) {
    if (!index_ || !vectors || count == 0) {
        return -1;
    }
    
    try {
        if (!is_trained() && index_type_ == "IVFFlat") {
            train(vectors, count);
        }
        
        if (ids) {
            index_->add_with_ids(count, vectors, ids);
        } else {
            index_->add(count, vectors);
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding vectors: " << e.what() << std::endl;
        return -2;
    }
}

std::vector<SearchResult> FAISSWrapper::search(const float* query, size_t k) {
    std::vector<SearchResult> results;
    
    if (!index_ || !query || k == 0) {
        return results;
    }
    
    try {
        std::vector<float> distances(k);
        std::vector<faiss::idx_t> labels(k);
        
        // TODO: Add batch search support for multiple queries
        // TODO: Implement search parameter tuning (nprobe for IVF indices)
        // TODO: Add support for range search and filtered search
        // TODO: Implement search result filtering and post-processing
        index_->search(1, query, k, distances.data(), labels.data());
        
        results.reserve(k);
        for (size_t i = 0; i < k; ++i) {
            if (labels[i] >= 0) {
                results.push_back({labels[i], distances[i]});
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error during search: " << e.what() << std::endl;
    }
    
    return results;
}

void FAISSWrapper::train(const float* training_data, size_t count) {
    if (!index_ || !training_data || count == 0) {
        return;
    }
    
    try {
        if (!index_->is_trained) {
            // TODO: Make training size adaptive based on index type and dataset characteristics
            // TODO: Implement progressive training for very large datasets
            // TODO: Add training quality validation and convergence metrics
            size_t training_size = std::min(count, size_t(100000));
            index_->train(training_size, training_data);
        }
        trained_ = true;
    } catch (const std::exception& e) {
        std::cerr << "Error training index: " << e.what() << std::endl;
    }
}

bool FAISSWrapper::is_trained() const {
    return index_ && (index_->is_trained || trained_);
}

std::vector<uint8_t> FAISSWrapper::serialize() const {
    std::vector<uint8_t> data;
    
    if (!index_) {
        return data;
    }
    
    // TODO: Add index metadata serialization (index type, parameters, creation time)
    // TODO: Implement compression for large index serialization
    // TODO: Add checksum validation for serialized data integrity
    // TODO: Support different serialization formats (binary, JSON metadata)
    
    try {
        std::stringstream ss;
        faiss::write_index(index_.get(), &ss);
        
        std::string str = ss.str();
        data.assign(str.begin(), str.end());
        
    } catch (const std::exception& e) {
        std::cerr << "Error serializing index: " << e.what() << std::endl;
        data.clear();
    }
    
    return data;
}

int FAISSWrapper::deserialize(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        return -1;
    }
    
    // TODO: Add version compatibility checking for different FAISS versions
    // TODO: Implement data validation and corruption detection
    // TODO: Add support for progressive loading of large indices
    // TODO: Implement fallback mechanisms for incompatible index formats
    
    try {
        std::string str(data.begin(), data.end());
        std::stringstream ss(str);
        
        auto loaded_index = faiss::read_index(&ss);
        if (!loaded_index) {
            return -2;
        }
        
        index_.reset(loaded_index);
        trained_ = index_->is_trained;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error deserializing index: " << e.what() << std::endl;
        return -3;
    }
}

size_t FAISSWrapper::get_ntotal() const {
    return index_ ? index_->ntotal : 0;
}

int FAISSWrapper::get_dimension() const {
    return dimension_;
}