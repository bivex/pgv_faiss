#include "pgv_faiss.h"
#include <iostream>

int main() {
    std::cout << "=== Database Connection Test ===" << std::endl;
    
    pgv_faiss_config_t config = {0};
    config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
    config.dimension = 128;
    config.use_gpu = 0;
    config.index_type = const_cast<char*>("IVFFlat");
    
    pgv_faiss_index_t* index = nullptr;
    
    std::cout << "Testing database connection..." << std::endl;
    int result = pgv_faiss_init(&config, &index);
    
    if (result == 0) {
        std::cout << "✅ Database connection successful!" << std::endl;
        std::cout << "✅ pgv_faiss library initialized properly!" << std::endl;
        
        pgv_faiss_destroy(index);
        std::cout << "✅ Cleanup completed" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Database connection failed with code: " << result << std::endl;
        return 1;
    }
}
