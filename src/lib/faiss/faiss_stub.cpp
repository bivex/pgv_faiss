#include "faiss_wrapper.h"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cstring>

// Forward declare FAISS types as stubs
namespace faiss {
    class Index {
    public:
        virtual ~Index() = default;
    };
}

// Stub implementation for FAISS functionality when FAISS is not available
struct FakeIndex : public faiss::Index {
    int dimension;
    std::vector<std::vector<float>> vectors;
    std::vector<int64_t> ids;
    
    FakeIndex(int dim) : dimension(dim) {}
};

FAISSWrapper::FAISSWrapper(int dimension, const std::string& index_type, 
                           bool use_gpu, int gpu_device)
    : dimension_(dimension), use_gpu_(false), gpu_device_(0), 
      index_type_(index_type), trained_(true) {
    
    std::cout << "Warning: Using stub FAISS implementation (FAISS not installed)" << std::endl;
    index_ = std::make_unique<FakeIndex>(dimension);
}

FAISSWrapper::~FAISSWrapper() = default;

int FAISSWrapper::add_vectors(const float* vectors, const int64_t* ids, size_t count) {
    FakeIndex* fake_idx = static_cast<FakeIndex*>(index_.get());
    
    for (size_t i = 0; i < count; ++i) {
        std::vector<float> vec(vectors + i * dimension_, vectors + (i + 1) * dimension_);
        fake_idx->vectors.push_back(vec);
        fake_idx->ids.push_back(ids[i]);
    }
    
    return 0; // Success
}

std::vector<SearchResult> FAISSWrapper::search(const float* query, size_t k) {
    FakeIndex* fake_idx = static_cast<FakeIndex*>(index_.get());
    std::vector<SearchResult> results;
    
    // TODO: Implement actual L2 distance calculation instead of random values
    // TODO: Add support for different distance metrics (cosine, dot product)
    // TODO: Implement proper brute-force search with actual vector comparisons
    // TODO: Consider using BLAS or other optimized libraries for distance calculations
    
    // Simple brute force search with random distances (stub implementation)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.1f, 10.0f);
    
    size_t count = std::min(k, fake_idx->ids.size());
    
    for (size_t i = 0; i < count; ++i) {
        if (i < fake_idx->ids.size()) {
            SearchResult result;
            result.id = fake_idx->ids[i];
            result.distance = dis(gen);
            results.push_back(result);
        }
    }
    
    // Sort by distance (ascending)
    std::sort(results.begin(), results.end(), 
              [](const SearchResult& a, const SearchResult& b) { 
                  return a.distance < b.distance; 
              });
    
    return results;
}

std::vector<uint8_t> FAISSWrapper::serialize() const {
    // Return a dummy serialization
    std::string dummy = "stub_faiss_index_v1.0";
    return std::vector<uint8_t>(dummy.begin(), dummy.end());
}

int FAISSWrapper::deserialize(const std::vector<uint8_t>& data) {
    // Accept any data for stub implementation
    return 0; // Success
}

void FAISSWrapper::train(const float* training_data, size_t count) {
    // Stub training always succeeds
    trained_ = true;
}

bool FAISSWrapper::is_trained() const {
    return trained_;
}

size_t FAISSWrapper::get_ntotal() const {
    FakeIndex* fake_idx = static_cast<FakeIndex*>(index_.get());
    return fake_idx ? fake_idx->ids.size() : 0;
}

int FAISSWrapper::get_dimension() const {
    return dimension_;
}

faiss::Index* FAISSWrapper::create_index(const std::string& index_type, int dimension) {
    return new FakeIndex(dimension);
}

void FAISSWrapper::setup_gpu_resources() {
    // Not used in stub implementation
}
