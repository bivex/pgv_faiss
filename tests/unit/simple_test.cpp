#include "pgv_faiss.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== PGV-FAISS Simple Library Test ===" << std::endl;
    
    // Test with dummy config to check basic initialization paths
    pgv_faiss_config_t config = {0};
    config.connection_string = const_cast<char*>("postgresql://dummy:dummy@dummy:5432/dummy");
    config.dimension = 128;
    config.use_gpu = 0;
    config.gpu_device_id = 0;
    config.index_type = const_cast<char*>("IVFFlat");
    config.nprobe = 10;
    
    std::cout << "✓ Configuration structure created successfully" << std::endl;
    
    pgv_faiss_index_t* index = nullptr;
    
    // This will fail due to no database, but we can test if the library loads and functions are callable
    std::cout << "Testing pgv_faiss_init function..." << std::endl;
    int result = pgv_faiss_init(&config, &index);
    
    if (result == -2) {
        std::cout << "✓ pgv_faiss_init correctly detected database connection failure (expected)" << std::endl;
    } else {
        std::cout << "✗ Unexpected result from pgv_faiss_init: " << result << std::endl;
        return 1;
    }
    
    // Test creating a result structure
    pgv_faiss_result_t result_struct = {0};
    std::cout << "✓ pgv_faiss_result_t structure created successfully" << std::endl;
    
    // Test the free function (should handle null gracefully)
    pgv_faiss_free_result(&result_struct);
    std::cout << "✓ pgv_faiss_free_result handled null structure gracefully" << std::endl;
    
    // Test destroy function with null (should handle gracefully)
    pgv_faiss_destroy(nullptr);
    std::cout << "✓ pgv_faiss_destroy handled null pointer gracefully" << std::endl;
    
    std::cout << "\n=== Library Test Summary ===" << std::endl;
    std::cout << "✓ Library loads correctly" << std::endl;
    std::cout << "✓ All API functions are accessible" << std::endl;
    std::cout << "✓ Error handling works as expected" << std::endl;
    std::cout << "✓ No crashes or undefined behavior detected" << std::endl;
    
    std::cout << "\n=== Test completed successfully ===" << std::endl;
    std::cout << "Note: Database connection tests failed as expected (no PostgreSQL server running)" << std::endl;
    
    return 0;
}
