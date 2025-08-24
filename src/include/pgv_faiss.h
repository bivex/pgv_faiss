#ifndef PGV_FAISS_H
#define PGV_FAISS_H

// TODO: Add API versioning macros (PGV_FAISS_VERSION_MAJOR, etc.)
// TODO: Add feature detection macros (PGV_FAISS_HAS_GPU, etc.)
// TODO: Add thread safety documentation and guarantees
// TODO: Consider adding async/callback-based API for large operations

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// TODO: Add more configuration options:
// - connection_timeout, retry_count, connection_pool_size
// - index_parameters (M for HNSW, ncentroids for IVF, etc.)
// - memory_limits, batch_sizes, threading_options
// - logging_level, progress_callback, error_callback
typedef struct pgv_faiss_config {
    char* connection_string;
    int dimension;
    int use_gpu;
    int gpu_device_id;
    char* index_type;
    int nprobe;
} pgv_faiss_config_t;

typedef struct pgv_faiss_index pgv_faiss_index_t;

typedef struct pgv_faiss_result {
    int64_t* ids;
    float* distances;
    size_t count;
} pgv_faiss_result_t;

// Core API functions
int pgv_faiss_init(pgv_faiss_config_t* config, pgv_faiss_index_t** index);
int pgv_faiss_add_vectors(pgv_faiss_index_t* index, const float* vectors, const int64_t* ids, size_t count);
int pgv_faiss_search(pgv_faiss_index_t* index, const float* query, size_t k, pgv_faiss_result_t* result);
int pgv_faiss_save_to_db(pgv_faiss_index_t* index, const char* table_name);
int pgv_faiss_load_from_db(pgv_faiss_index_t* index, const char* table_name);
void pgv_faiss_free_result(pgv_faiss_result_t* result);
void pgv_faiss_destroy(pgv_faiss_index_t* index);

// TODO: Add missing API functions:
// - pgv_faiss_get_stats() for index statistics
// - pgv_faiss_batch_search() for multiple queries
// - pgv_faiss_range_search() for radius-based search
// - pgv_faiss_remove_vectors() for vector deletion
// - pgv_faiss_update_vector() for vector modification
// - pgv_faiss_get_vector() for vector retrieval
// - pgv_faiss_validate_config() for configuration validation
// - pgv_faiss_get_version() for version information

#ifdef __cplusplus
}
#endif

#endif