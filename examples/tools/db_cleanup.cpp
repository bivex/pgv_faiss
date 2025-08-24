#include "pgv_faiss.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

// PostgreSQL connection for direct SQL operations
#include <libpq-fe.h>

class DatabaseCleanup {
private:
    PGconn* conn_;
    std::string connection_string_;

public:
    explicit DatabaseCleanup(const std::string& connection_string) 
        : conn_(nullptr), connection_string_(connection_string) {}

    ~DatabaseCleanup() {
        disconnect();
    }

    bool connect() {
        conn_ = PQconnectdb(connection_string_.c_str());
        
        if (PQstatus(conn_) != CONNECTION_OK) {
            std::cerr << "Connection failed: " << PQerrorMessage(conn_) << std::endl;
            PQfinish(conn_);
            conn_ = nullptr;
            return false;
        }
        
        return true;
    }

    void disconnect() {
        if (conn_) {
            PQfinish(conn_);
            conn_ = nullptr;
        }
    }

    bool execute_query(const std::string& query) {
        if (!conn_) return false;
        
        PGresult* result = PQexec(conn_, query.c_str());
        bool success = (PQresultStatus(result) == PGRES_COMMAND_OK);
        
        if (!success) {
            std::cerr << "Query failed: " << PQerrorMessage(conn_) << std::endl;
            std::cerr << "Query: " << query << std::endl;
        }
        
        PQclear(result);
        return success;
    }

    std::vector<std::string> get_tables() {
        std::vector<std::string> tables;
        
        if (!conn_) return tables;
        
        const char* query = "SELECT tablename FROM pg_tables WHERE schemaname = 'public'";
        PGresult* result = PQexec(conn_, query);
        
        if (PQresultStatus(result) == PGRES_TUPLES_OK) {
            int rows = PQntuples(result);
            for (int i = 0; i < rows; ++i) {
                const char* table_name = PQgetvalue(result, i, 0);
                tables.push_back(std::string(table_name));
            }
        }
        
        PQclear(result);
        return tables;
    }

    bool clear_table(const std::string& table_name) {
        std::string query = "DROP TABLE IF EXISTS " + table_name + " CASCADE";
        std::cout << "Dropping table: " << table_name << std::endl;
        return execute_query(query);
    }

    bool clear_all_vector_tables() {
        auto tables = get_tables();
        bool success = true;
        
        std::cout << "=== Clearing Vector Tables ===" << std::endl;
        
        for (const auto& table : tables) {
            // Skip system tables and focus on potential vector tables
            if (table.find("test") != std::string::npos ||
                table.find("vector") != std::string::npos ||
                table.find("faiss") != std::string::npos ||
                table.find("index") != std::string::npos ||
                table.find("benchmark") != std::string::npos ||
                table.find("sample") != std::string::npos ||
                table.find("embedding") != std::string::npos) {
                
                if (!clear_table(table)) {
                    success = false;
                }
            }
        }
        
        return success;
    }

    bool clear_faiss_indices() {
        auto tables = get_tables();
        bool success = true;
        
        std::cout << "=== Clearing FAISS Index Tables ===" << std::endl;
        
        for (const auto& table : tables) {
            if (table.find("_faiss_index") != std::string::npos) {
                if (!clear_table(table)) {
                    success = false;
                }
            }
        }
        
        return success;
    }

    void show_database_stats() {
        std::cout << "=== Database Statistics ===" << std::endl;
        
        // Show table count
        auto tables = get_tables();
        std::cout << "Total tables: " << tables.size() << std::endl;
        
        // Show database size
        const char* size_query = "SELECT pg_size_pretty(pg_database_size(current_database()))";
        PGresult* result = PQexec(conn_, size_query);
        
        if (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0) {
            const char* size = PQgetvalue(result, 0, 0);
            std::cout << "Database size: " << size << std::endl;
        }
        
        PQclear(result);
        
        // List remaining tables
        if (!tables.empty()) {
            std::cout << "Remaining tables:" << std::endl;
            for (const auto& table : tables) {
                std::cout << "  - " << table << std::endl;
            }
        } else {
            std::cout << "No user tables remaining." << std::endl;
        }
    }

    bool vacuum_database() {
        std::cout << "=== Vacuuming Database ===" << std::endl;
        return execute_query("VACUUM FULL");
    }
};

void print_usage(const char* program_name) {
    std::cout << "Database Cleanup Tool for pgv_faiss" << std::endl;
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --all          Clear all vector tables and FAISS indices" << std::endl;
    std::cout << "  --vectors      Clear only vector tables" << std::endl;
    std::cout << "  --indices      Clear only FAISS index tables" << std::endl;
    std::cout << "  --stats        Show database statistics" << std::endl;
    std::cout << "  --vacuum       Vacuum database after cleanup" << std::endl;
    std::cout << "  --help         Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " --all          # Clear everything" << std::endl;
    std::cout << "  " << program_name << " --vectors --vacuum  # Clear vectors and vacuum" << std::endl;
    std::cout << "  " << program_name << " --stats        # Show database info" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== pgv_faiss Database Cleanup Tool ===" << std::endl;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Parse command line arguments
    bool clear_all = false;
    bool clear_vectors = false;
    bool clear_indices = false;
    bool show_stats = false;
    bool vacuum = false;
    bool show_help = false;
    
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--all") == 0) {
            clear_all = true;
        } else if (strcmp(argv[i], "--vectors") == 0) {
            clear_vectors = true;
        } else if (strcmp(argv[i], "--indices") == 0) {
            clear_indices = true;
        } else if (strcmp(argv[i], "--stats") == 0) {
            show_stats = true;
        } else if (strcmp(argv[i], "--vacuum") == 0) {
            vacuum = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            show_help = true;
        } else {
            std::cerr << "Unknown option: " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (show_help) {
        print_usage(argv[0]);
        return 0;
    }
    
    // Database connection configuration
    std::string connection_string = "postgresql://pgvuser:pgvpass@localhost:5432/vectordb";
    
    DatabaseCleanup cleanup(connection_string);
    
    if (!cleanup.connect()) {
        std::cerr << "Failed to connect to database!" << std::endl;
        std::cerr << "Make sure PostgreSQL is running and the database is accessible." << std::endl;
        return 1;
    }
    
    std::cout << "Connected to database successfully!" << std::endl;
    
    // Show initial stats
    if (show_stats || clear_all || clear_vectors || clear_indices) {
        std::cout << std::endl;
        std::cout << "=== Before Cleanup ===" << std::endl;
        cleanup.show_database_stats();
        std::cout << std::endl;
    }
    
    bool cleanup_performed = false;
    
    // Perform cleanup operations
    if (clear_all) {
        std::cout << "Performing complete database cleanup..." << std::endl;
        cleanup.clear_all_vector_tables();
        cleanup.clear_faiss_indices();
        cleanup_performed = true;
    } else {
        if (clear_vectors) {
            cleanup.clear_all_vector_tables();
            cleanup_performed = true;
        }
        
        if (clear_indices) {
            cleanup.clear_faiss_indices();
            cleanup_performed = true;
        }
    }
    
    // Vacuum if requested
    if (vacuum && cleanup_performed) {
        std::cout << std::endl;
        cleanup.vacuum_database();
    }
    
    // Show final stats
    if (cleanup_performed || show_stats) {
        std::cout << std::endl;
        std::cout << "=== After Cleanup ===" << std::endl;
        cleanup.show_database_stats();
    }
    
    if (cleanup_performed) {
        std::cout << std::endl;
        std::cout << "✅ Database cleanup completed successfully!" << std::endl;
        std::cout << "The database is now ready for fresh benchmark runs." << std::endl;
    }
    
    return 0;
}