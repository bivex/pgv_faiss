#include "pgv_connection.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace pgvector {

PGVConnection::PGVConnection(const std::string& connection_string) 
    : conn_string_(connection_string), conn_(nullptr) {
}

PGVConnection::~PGVConnection() {
    disconnect();
}

bool PGVConnection::connect() {
    conn_ = PQconnectdb(conn_string_.c_str());
    
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn_) << std::endl;
        PQfinish(conn_);
        conn_ = nullptr;
        return false;
    }
    
    create_extension();
    return true;
}

void PGVConnection::disconnect() {
    if (conn_) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

bool PGVConnection::is_connected() const {
    return conn_ && PQstatus(conn_) == CONNECTION_OK;
}

bool PGVConnection::create_extension() {
    return execute_query("CREATE EXTENSION IF NOT EXISTS vector");
}

bool PGVConnection::create_table(const std::string& table_name, int dimension) {
    std::ostringstream query;
    query << "CREATE TABLE IF NOT EXISTS " << table_name 
          << " (id bigserial PRIMARY KEY, embedding vector(" << dimension << "))";
    return execute_query(query.str());
}

bool PGVConnection::insert_vector(const std::string& table_name, int64_t id, const std::vector<float>& vector) {
    std::ostringstream query;
    query << "INSERT INTO " << table_name << " (id, embedding) VALUES (" << id << ", '[";
    
    for (size_t i = 0; i < vector.size(); ++i) {
        if (i > 0) query << ",";
        query << std::fixed << std::setprecision(6) << vector[i];
    }
    query << "]')";
    
    return execute_query(query.str());
}

bool PGVConnection::batch_insert_vectors(const std::string& table_name, 
                                       const std::vector<int64_t>& ids, 
                                       const std::vector<std::vector<float>>& vectors) {
    if (ids.size() != vectors.size()) return false;
    
    std::ostringstream query;
    query << "INSERT INTO " << table_name << " (id, embedding) VALUES ";
    
    for (size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) query << ", ";
        query << "(" << ids[i] << ", '[";
        
        for (size_t j = 0; j < vectors[i].size(); ++j) {
            if (j > 0) query << ",";
            query << std::fixed << std::setprecision(6) << vectors[i][j];
        }
        query << "]')";
    }
    
    return execute_query(query.str());
}

std::vector<std::pair<int64_t, float>> PGVConnection::similarity_search(
    const std::string& table_name, const std::vector<float>& query, size_t k) {
    
    // TODO: Use parameterized queries to prevent SQL injection
    // TODO: Add support for different distance operators (<->, <#>, <=>)  
    // TODO: Implement connection pooling for high-throughput scenarios
    // TODO: Add query caching and prepared statement optimization
    std::ostringstream sql;
    sql << "SELECT id, embedding <-> '[";
    for (size_t i = 0; i < query.size(); ++i) {
        if (i > 0) sql << ",";
        sql << std::fixed << std::setprecision(6) << query[i];
    }
    sql << "]' AS distance FROM " << table_name 
        << " ORDER BY embedding <-> '[";
    for (size_t i = 0; i < query.size(); ++i) {
        if (i > 0) sql << ",";
        sql << std::fixed << std::setprecision(6) << query[i];
    }
    sql << "]' LIMIT " << k;
    
    auto result = execute_query_result(sql.str());
    std::vector<std::pair<int64_t, float>> results;
    
    if (result) {
        int rows = PQntuples(result);
        for (int i = 0; i < rows; ++i) {
            int64_t id = std::stoll(PQgetvalue(result, i, 0));
            float distance = std::stof(PQgetvalue(result, i, 1));
            results.emplace_back(id, distance);
        }
        PQclear(result);
    }
    
    return results;
}

int PGVConnection::save_index(const std::string& table_name, const std::vector<uint8_t>& index_data) {
    std::string index_table = table_name + "_faiss_index";
    
    // TODO: Add versioning support for index storage
    // TODO: Implement compression for large index data
    // TODO: Add metadata storage (index type, parameters, creation time)
    // TODO: Implement transactional safety for index updates
    execute_query("CREATE TABLE IF NOT EXISTS " + index_table + " (id SERIAL PRIMARY KEY, index_data BYTEA)");
    execute_query("DELETE FROM " + index_table);
    
    size_t escaped_length;
    unsigned char* escaped_data = PQescapeByteaConn(conn_, index_data.data(), index_data.size(), &escaped_length);
    
    if (!escaped_data) return -1;
    
    std::string query = "INSERT INTO " + index_table + " (index_data) VALUES ('" + 
                       std::string(reinterpret_cast<char*>(escaped_data)) + "')";
    
    PQfreemem(escaped_data);
    
    return execute_query(query) ? 0 : -1;
}

std::vector<uint8_t> PGVConnection::load_index(const std::string& table_name) {
    std::string index_table = table_name + "_faiss_index";
    std::string query = "SELECT index_data FROM " + index_table + " ORDER BY id DESC LIMIT 1";
    
    auto result = execute_query_result(query);
    std::vector<uint8_t> data;
    
    if (result && PQntuples(result) > 0) {
        size_t length;
        unsigned char* raw_data = PQunescapeBytea(
            reinterpret_cast<unsigned char*>(PQgetvalue(result, 0, 0)), &length);
        
        if (raw_data) {
            data.assign(raw_data, raw_data + length);
            PQfreemem(raw_data);
        }
        PQclear(result);
    }
    
    return data;
}

bool PGVConnection::execute_query(const std::string& query) {
    if (!is_connected()) return false;
    
    PGresult* result = PQexec(conn_, query.c_str());
    bool success = PQresultStatus(result) == PGRES_COMMAND_OK;
    
    if (!success) {
        std::cerr << "Query failed: " << PQerrorMessage(conn_) << std::endl;
        std::cerr << "Query: " << query << std::endl;
    }
    
    PQclear(result);
    return success;
}

PGresult* PGVConnection::execute_query_result(const std::string& query) {
    if (!is_connected()) return nullptr;
    
    PGresult* result = PQexec(conn_, query.c_str());
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::cerr << "Query failed: " << PQerrorMessage(conn_) << std::endl;
        PQclear(result);
        return nullptr;
    }
    
    return result;
}

} // namespace pgvector
