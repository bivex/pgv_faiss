#ifndef FAISS_WRAPPER_H
#define FAISS_WRAPPER_H

#include <vector>
#include <memory>
#include <string>

namespace faiss {
    class Index;
    class IndexIVFFlat;
}

struct SearchResult {
    int64_t id;
    float distance;
};

class FAISSWrapper {
public:
    FAISSWrapper(int dimension, const std::string& index_type = "IVFFlat", 
                 bool use_gpu = false, int gpu_device = 0);
    ~FAISSWrapper();

    int add_vectors(const float* vectors, const int64_t* ids, size_t count);
    std::vector<SearchResult> search(const float* query, size_t k);
    
    std::vector<uint8_t> serialize() const;
    int deserialize(const std::vector<uint8_t>& data);
    
    void train(const float* training_data, size_t count);
    bool is_trained() const;
    
    size_t get_ntotal() const;
    int get_dimension() const;

private:
    std::unique_ptr<faiss::Index> index_;
    int dimension_;
    bool use_gpu_;
    int gpu_device_;
    std::string index_type_;
    bool trained_;
    
    faiss::Index* create_index(const std::string& index_type, int dimension);
    void setup_gpu_resources();
};

#endif