#ifndef PGV_CONNECTION_H
#define PGV_CONNECTION_H

// TODO: Add connection pooling support for high-throughput applications
// TODO: Add async operation support with callbacks or futures
// TODO: Add transaction management and rollback capabilities
// TODO: Add connection retry logic with exponential backoff

#include <string>
#include <vector>
#include <libpq-fe.h>

namespace pgvector {

class PGVConnection {
public:
    explicit PGVConnection(const std::string& connection_string);
    ~PGVConnection();

    bool connect();
    void disconnect();
    bool is_connected() const;

    bool create_extension();
    bool create_table(const std::string& table_name, int dimension);
    bool insert_vector(const std::string& table_name, int64_t id, const std::vector<float>& vector);
    bool batch_insert_vectors(const std::string& table_name, const std::vector<int64_t>& ids, const std::vector<std::vector<float>>& vectors);
    
    std::vector<std::pair<int64_t, float>> similarity_search(const std::string& table_name, const std::vector<float>& query, size_t k);
    
    int save_index(const std::string& table_name, const std::vector<uint8_t>& index_data);
    std::vector<uint8_t> load_index(const std::string& table_name);
    
    // Additional methods for vector operations
    std::vector<std::vector<float>> fetch_vectors(const std::string& table_name, int limit = 0);
    bool store_vectors(const std::string& table_name, const std::vector<std::vector<float>>& vectors, const std::vector<int64_t>& ids);
    
    // TODO: Add missing methods:
    // - delete_vector(table_name, id) for vector removal
    // - update_vector(table_name, id, vector) for vector modification
    // - get_table_stats(table_name) for table statistics
    // - vacuum_table(table_name) for maintenance operations
    // - create_index(table_name, index_type) for pgvector indices
    // - batch_delete_vectors(table_name, ids) for bulk deletion
    // - get_vector_by_id(table_name, id) for single vector retrieval

private:
    std::string conn_string_;
    PGconn* conn_;
    
    bool execute_query(const std::string& query);
    PGresult* execute_query_result(const std::string& query);
    std::vector<float> parse_vector_string(const std::string& vector_str);
};

} // namespace pgvector

#endif
