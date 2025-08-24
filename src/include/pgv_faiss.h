#ifndef PGV_FAISS_H
#define PGV_FAISS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

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

int pgv_faiss_init(pgv_faiss_config_t* config, pgv_faiss_index_t** index);
int pgv_faiss_add_vectors(pgv_faiss_index_t* index, const float* vectors, const int64_t* ids, size_t count);
int pgv_faiss_search(pgv_faiss_index_t* index, const float* query, size_t k, pgv_faiss_result_t* result);
int pgv_faiss_save_to_db(pgv_faiss_index_t* index, const char* table_name);
int pgv_faiss_load_from_db(pgv_faiss_index_t* index, const char* table_name);
void pgv_faiss_free_result(pgv_faiss_result_t* result);
void pgv_faiss_destroy(pgv_faiss_index_t* index);

#ifdef __cplusplus
}
#endif

#endif