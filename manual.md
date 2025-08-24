# pgv_faiss Manual

## Table of Contents
1. [Overview](#overview)
2. [System Requirements](#system-requirements)
3. [Installation Guide](#installation-guide)
4. [Database Setup](#database-setup)
5. [Building the Project](#building-the-project)
6. [Configuration](#configuration)
7. [Usage Examples](#usage-examples)
8. [API Reference](#api-reference)
9. [Performance Guidelines](#performance-guidelines)
10. [Troubleshooting](#troubleshooting)
11. [Development](#development)

## Overview

pgv_faiss is a C/C++ library that integrates Facebook AI Similarity Search (FAISS) with PostgreSQL vector data handling, enabling efficient similarity search operations on vector embeddings stored in PostgreSQL. The library provides a unified API for vector storage, retrieval, and approximate nearest neighbor (ANN) searches.

### Key Features
- **High-Performance Vector Search**: Leverages FAISS algorithms (IVFFlat, HNSW, Flat)
- **PostgreSQL Integration**: Seamless integration with PostgreSQL and pgvector extension
- **C/C++ API**: Native performance with C linkage for broad compatibility
- **Flexible Indexing**: Support for multiple index types and configurations
- **Database Persistence**: Store and retrieve FAISS indices in PostgreSQL
- **Stub Implementation**: Fallback functionality when FAISS is not available

## System Requirements

### Minimum Requirements
- **Operating System**: Linux (Debian/Ubuntu recommended)
- **Compiler**: GCC 7+ or Clang 5+ with C++17 support
- **CMake**: Version 3.16 or higher
- **PostgreSQL**: Version 12+ (Version 15 recommended)
- **Memory**: 4GB RAM minimum (8GB+ recommended for large datasets)

### Dependencies
- `libpq-dev` - PostgreSQL client library development headers
- `pkg-config` - Package configuration tool
- `build-essential` - Compilation tools (GCC, make, etc.)
- `git` - For source code management
- `postgresql-server-dev-15` - PostgreSQL server development headers

### Optional Dependencies
- **NVIDIA CUDA Toolkit** - For GPU-accelerated operations (optional)
- **FAISS Library** - For full FAISS functionality (uses stub if not available)

## Installation Guide

### 1. Install System Dependencies

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y postgresql postgresql-contrib build-essential cmake pkg-config git
```

#### PostgreSQL Development Headers:
```bash
sudo apt install -y postgresql-server-dev-15
```

### 2. PostgreSQL Setup

#### Start PostgreSQL Service:
```bash
sudo systemctl start postgresql
sudo systemctl enable postgresql
```

#### Verify PostgreSQL is Running:
```bash
sudo systemctl status postgresql
netstat -tlnp | grep 5432
```

### 3. Install pgvector Extension

#### Clone and Build pgvector:
```bash
cd /tmp
git clone https://github.com/pgvector/pgvector.git
cd pgvector
make
sudo make install
```

#### Restart PostgreSQL:
```bash
sudo systemctl restart postgresql
```

## Database Setup

### 1. Create Database and User

#### Connect as postgres user:
```bash
sudo -u postgres psql
```

#### Create user and database:
```sql
CREATE USER pgvuser WITH CREATEDB SUPERUSER PASSWORD 'pgvpass';
CREATE DATABASE vectordb OWNER pgvuser;
\q
```

### 2. Enable pgvector Extension

#### Connect to the database:
```bash
PGPASSWORD=pgvpass psql -h localhost -U pgvuser -d vectordb
```

#### Create the extension:
```sql
CREATE EXTENSION IF NOT EXISTS vector;
\dx
\q
```

### 3. Verify Database Connection

```bash
PGPASSWORD=pgvpass psql -h localhost -U pgvuser -d vectordb -c "SELECT version();"
```

## Building the Project

### 1. Clone the Repository

```bash
git clone <repository-url>
cd pgv_faiss
```

### 2. Create Build Directory

```bash
mkdir build
cd build
```

### 3. Configure with CMake

#### Basic Configuration:
```bash
cmake .. \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_TESTS=OFF \
    -DWITH_GPU=OFF \
    -DCMAKE_CXX_STANDARD=17
```

#### Configuration Options:
- `BUILD_SHARED_LIBS=ON` - Build shared library (.so)
- `BUILD_EXAMPLES=ON` - Build example programs
- `BUILD_TESTS=ON` - Build test suite
- `WITH_GPU=ON` - Enable GPU support (requires CUDA)
- `CMAKE_BUILD_TYPE=Debug` - Debug build (default: Release)

### 4. Build the Project

```bash
make -j$(nproc)
```

### 5. Verify Build

```bash
ls -la src/lib/libpgv_faiss.so
ls -la examples/
```

## Configuration

### Database Connection Configuration

The default connection string format is:
```
postgresql://pgvuser:pgvpass@localhost:5432/vectordb
```

#### Connection String Components:
- **Username**: `pgvuser`
- **Password**: `pgvpass`
- **Host**: `localhost`
- **Port**: `5432`
- **Database**: `vectordb`

### Environment Variables

Set the library path for runtime:
```bash
export LD_LIBRARY_PATH=/path/to/pgv_faiss/build/src/lib:$LD_LIBRARY_PATH
```

### pgv_faiss Configuration Structure

```c
typedef struct pgv_faiss_config {
    char* connection_string;    // PostgreSQL connection string
    int dimension;             // Vector dimension (e.g., 128, 256, 512)
    int use_gpu;              // 0 = CPU only, 1 = GPU enabled
    int gpu_device_id;        // GPU device ID (if use_gpu = 1)
    char* index_type;         // "IVFFlat", "HNSW", or "Flat"
    int nprobe;              // Number of clusters to search (for IVF indices)
} pgv_faiss_config_t;
```

## Usage Examples

### 1. Basic Vector Operations

```c
#include "pgv_faiss.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Configure the index
    pgv_faiss_config_t config = {0};
    config.connection_string = "postgresql://pgvuser:pgvpass@localhost:5432/vectordb";
    config.dimension = 128;
    config.use_gpu = 0;
    config.index_type = "IVFFlat";
    config.nprobe = 10;
    
    pgv_faiss_index_t* index = NULL;
    
    // Initialize the index
    int result = pgv_faiss_init(&config, &index);
    if (result != 0) {
        printf("Failed to initialize index: %d\n", result);
        return 1;
    }
    
    // Prepare sample vectors
    const int num_vectors = 1000;
    const int dimension = 128;
    float* vectors = malloc(num_vectors * dimension * sizeof(float));
    int64_t* ids = malloc(num_vectors * sizeof(int64_t));
    
    // Generate random vectors (replace with your data)
    for (int i = 0; i < num_vectors; i++) {
        ids[i] = i;
        for (int j = 0; j < dimension; j++) {
            vectors[i * dimension + j] = (float)rand() / RAND_MAX;
        }
    }
    
    // Add vectors to index
    result = pgv_faiss_add_vectors(index, vectors, ids, num_vectors);
    if (result != 0) {
        printf("Failed to add vectors: %d\n", result);
        goto cleanup;
    }
    
    // Perform similarity search
    float query[128];
    for (int i = 0; i < 128; i++) {
        query[i] = (float)rand() / RAND_MAX;
    }
    
    pgv_faiss_result_t search_result = {0};
    result = pgv_faiss_search(index, query, 10, &search_result);
    if (result == 0) {
        printf("Found %zu similar vectors:\n", search_result.count);
        for (size_t i = 0; i < search_result.count; i++) {
            printf("ID: %ld, Distance: %f\n", 
                   search_result.ids[i], search_result.distances[i]);
        }
    }
    
    // Save index to database
    pgv_faiss_save_to_db(index, "my_index");
    
    // Cleanup
    pgv_faiss_free_result(&search_result);
    
cleanup:
    free(vectors);
    free(ids);
    pgv_faiss_destroy(index);
    return 0;
}
```

### 2. Compilation Example

```bash
gcc -std=c17 -I/path/to/pgv_faiss/src/include \
    -L/path/to/pgv_faiss/build/src/lib \
    -lpgv_faiss -lpq \
    my_program.c -o my_program
```

### 3. C++ Usage

```cpp
#include "pgv_faiss.h"
#include <iostream>
#include <vector>

int main() {
    pgv_faiss_config_t config = {0};
    config.connection_string = const_cast<char*>("postgresql://pgvuser:pgvpass@localhost:5432/vectordb");
    config.dimension = 256;
    config.use_gpu = 0;
    config.index_type = const_cast<char*>("HNSW");
    
    pgv_faiss_index_t* index = nullptr;
    
    if (pgv_faiss_init(&config, &index) == 0) {
        std::cout << "Index initialized successfully!" << std::endl;
        
        // Your vector operations here...
        
        pgv_faiss_destroy(index);
    }
    
    return 0;
}
```

## API Reference

### Core Functions

#### pgv_faiss_init
```c
int pgv_faiss_init(pgv_faiss_config_t* config, pgv_faiss_index_t** index);
```
Initialize a new pgv_faiss index with the given configuration.

**Parameters:**
- `config`: Configuration structure
- `index`: Output parameter for the created index

**Returns:** 0 on success, negative error code on failure

#### pgv_faiss_add_vectors
```c
int pgv_faiss_add_vectors(pgv_faiss_index_t* index, const float* vectors, 
                          const int64_t* ids, size_t count);
```
Add vectors to the index.

**Parameters:**
- `index`: The pgv_faiss index
- `vectors`: Array of vectors (flattened: count * dimension elements)
- `ids`: Array of vector IDs
- `count`: Number of vectors to add

**Returns:** 0 on success, negative error code on failure

#### pgv_faiss_search
```c
int pgv_faiss_search(pgv_faiss_index_t* index, const float* query, 
                     size_t k, pgv_faiss_result_t* result);
```
Perform similarity search for the k nearest neighbors.

**Parameters:**
- `index`: The pgv_faiss index
- `query`: Query vector (dimension elements)
- `k`: Number of nearest neighbors to find
- `result`: Output structure for search results

**Returns:** 0 on success, negative error code on failure

#### pgv_faiss_save_to_db
```c
int pgv_faiss_save_to_db(pgv_faiss_index_t* index, const char* table_name);
```
Save the FAISS index to PostgreSQL database.

#### pgv_faiss_load_from_db
```c
int pgv_faiss_load_from_db(pgv_faiss_index_t* index, const char* table_name);
```
Load a previously saved FAISS index from PostgreSQL database.

#### pgv_faiss_free_result
```c
void pgv_faiss_free_result(pgv_faiss_result_t* result);
```
Free memory allocated for search results.

#### pgv_faiss_destroy
```c
void pgv_faiss_destroy(pgv_faiss_index_t* index);
```
Destroy the index and free all associated memory.

### Data Structures

#### pgv_faiss_result_t
```c
typedef struct pgv_faiss_result {
    int64_t* ids;           // Array of result IDs
    float* distances;       // Array of distances
    size_t count;          // Number of results
} pgv_faiss_result_t;
```

### Error Codes

- `0`: Success
- `-1`: Invalid parameters
- `-2`: Database connection failed
- `-3`: Memory allocation failed
- `-4`: Index operation failed

## Performance Guidelines

### Vector Dimensions
- **Small**: 50-128 dimensions - Optimal for most applications
- **Medium**: 256-512 dimensions - Good performance with proper indexing
- **Large**: 1024+ dimensions - Consider dimensionality reduction

### Index Type Selection

#### Flat Index
- **Best for**: Small datasets (< 10,000 vectors)
- **Pros**: Exact results, simple
- **Cons**: Linear search time O(n)

#### IVFFlat Index
- **Best for**: Medium datasets (10,000 - 1,000,000 vectors)
- **Pros**: Good balance of speed and accuracy
- **Cons**: Requires training data

#### HNSW Index
- **Best for**: Large datasets (> 100,000 vectors)
- **Pros**: Excellent search speed, good accuracy
- **Cons**: Higher memory usage

### Memory Usage Optimization

```c
// For large datasets, consider batch processing
const size_t batch_size = 10000;
for (size_t i = 0; i < total_vectors; i += batch_size) {
    size_t current_batch = min(batch_size, total_vectors - i);
    pgv_faiss_add_vectors(index, &vectors[i * dimension], 
                          &ids[i], current_batch);
}
```

## Troubleshooting

### Common Issues

#### 1. Database Connection Failed
```
Error: connection to server at "localhost", port 5432 failed
```

**Solutions:**
- Verify PostgreSQL is running: `sudo systemctl status postgresql`
- Check connection parameters in connection string
- Ensure user has proper permissions
- Verify database exists: `psql -l`

#### 2. pgvector Extension Not Found
```
Error: extension "vector" is not available
```

**Solutions:**
- Install pgvector from source (see Installation Guide)
- Restart PostgreSQL after installation
- Verify installation: `SELECT * FROM pg_available_extensions WHERE name = 'vector';`

#### 3. Library Loading Issues
```
Error: libpgv_faiss.so: cannot open shared object file
```

**Solutions:**
- Set LD_LIBRARY_PATH: `export LD_LIBRARY_PATH=/path/to/lib:$LD_LIBRARY_PATH`
- Use absolute paths in compilation
- Check library exists: `ls -la /path/to/libpgv_faiss.so`

#### 4. Compilation Errors
```
Error: undefined reference to 'pgv_faiss_init'
```

**Solutions:**
- Link with `-lpgv_faiss -lpq`
- Include correct header paths: `-I/path/to/include`
- Verify library path: `-L/path/to/lib`

#### 5. Database Cleanup Tool Issues
```
Error: Database cleanup tool not found
```

**Solutions:**
- Build the cleanup tool: `make db_cleanup`
- Check if tool exists: `ls -la examples/db_cleanup`
- Use the cleanup script: `./scripts/cleanup_db.sh`

```
Error: Permission denied when running cleanup script
```

**Solutions:**
- Make script executable: `chmod +x scripts/cleanup_db.sh`
- Run with bash: `bash scripts/cleanup_db.sh all`

### Debug Mode

Compile with debug information:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

Run with gdb:
```bash
gdb --args ./my_program
```

### Logging

Enable verbose logging (if supported):
```c
// Set environment variable
setenv("PGV_FAISS_DEBUG", "1", 1);
```

## Development

### Project Structure
```
pgv_faiss/
├── src/
│   ├── include/
│   │   └── pgv_faiss.h          # Public API header
│   └── lib/
│       ├── core/
│       │   └── pgv_faiss_core.cpp    # Main API implementation
│       ├── pgvector/
│       │   ├── pgv_connection.h      # PostgreSQL connection
│       │   ├── pgv_connection.cpp
│       │   └── pgv_operations.cpp    # Vector operations
│       └── faiss/
│           ├── faiss_wrapper.cpp     # FAISS integration
│           └── faiss_stub.cpp        # Stub implementation
├── examples/
│   ├── basic/basic_usage.cpp         # Basic usage example
│   ├── advanced/advanced_usage.cpp   # Advanced features
│   ├── advanced/gpu_example.cpp      # GPU acceleration example
│   ├── benchmarks/benchmark.cpp      # Performance testing
│   └── tools/
│       └── db_cleanup.cpp            # Database cleanup tool
├── tests/
│   ├── unit/
│   │   ├── database_test.cpp         # Database connection test
│   │   └── simple_test.cpp           # Basic library test
│   ├── CMakeLists.txt                # Test build configuration
│   └── README.md                     # Test documentation
├── scripts/
│   ├── cleanup_db.sh                 # Database cleanup script
│   ├── clean_build.sh                # Build cleanup script
│   └── README.md                     # Script documentation
├── build/                            # Build output directory
├── CMakeLists.txt                    # Main CMake configuration
├── .gitignore                        # Git ignore rules
└── manual.md                         # This file
```

### Adding New Features

1. **Extend the API**: Add new functions to `pgv_faiss.h`
2. **Implement Core Logic**: Add implementation to `pgv_faiss_core.cpp`
3. **Update CMake**: Modify `CMakeLists.txt` if needed
4. **Add Tests**: Create test cases for new functionality
5. **Update Documentation**: Update this manual

### Testing

Run the built-in examples:
```bash
cd build
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/basic_example
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/advanced_example
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/benchmark_example
```

### Unit Testing

The project includes comprehensive unit tests for library functionality:

#### Building Tests:
```bash
cd build
cmake .. -DBUILD_TESTS=ON
make
```

#### Running Individual Tests:
```bash
# Set library path
export LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH

# Simple library test (no database required)
./tests/simple_test

# Database connection test (requires PostgreSQL)
./tests/database_test
```

#### Running All Tests:
```bash
# Using CMake target
make run_tests

# Using CTest
ctest
```

#### Test Descriptions:
- **Simple Test**: Verifies library loads correctly, API functions are accessible, and error handling works
- **Database Test**: Tests actual PostgreSQL database connectivity and library initialization

For detailed testing information, see [`tests/README.md`](tests/README.md).

### Database Cleanup Tool

For benchmarking and testing, a database cleanup tool is provided to clear vector tables and FAISS indices:

#### Building the Cleanup Tool:
```bash
cd build
make db_cleanup
```

#### Using the Cleanup Tool:
```bash
# Complete cleanup
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/db_cleanup --all

# Clear only vector tables
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/db_cleanup --vectors

# Clear only FAISS indices
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/db_cleanup --indices

# Show database statistics
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/db_cleanup --stats

# Full cleanup with vacuum
LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH ./examples/db_cleanup --all --vacuum
```

#### Using the Cleanup Script (Easier):
```bash
# Make script executable
chmod +x scripts/cleanup_db.sh

# Run cleanup operations
./scripts/cleanup_db.sh all      # Complete cleanup
./scripts/cleanup_db.sh vectors  # Clear vector tables only
./scripts/cleanup_db.sh indices  # Clear FAISS indices only
./scripts/cleanup_db.sh stats    # Show database stats
./scripts/cleanup_db.sh vacuum   # Full cleanup with vacuum
```

**Note**: The cleanup tool is particularly useful before running benchmarks to ensure clean, reproducible results.

### Build Cleanup Script

For development and CI/CD workflows, a build cleanup script is provided to remove temporary files and build artifacts:

#### Using the Build Cleanup Script:
```bash
# Make script executable
chmod +x scripts/clean_build.sh

# Clean all build files and artifacts
./scripts/clean_build.sh all

# Clean only build directory
./scripts/clean_build.sh build

# Clean temporary files (CMake cache, logs)
./scripts/clean_build.sh temp

# Clean benchmark and test results
./scripts/clean_build.sh results

# Clean backup files (.bak, .orig, ~)
./scripts/clean_build.sh backup

# Force clean everything (including git-ignored files)
./scripts/clean_build.sh force

# Show help
./scripts/clean_build.sh help
```

#### What Gets Cleaned:
- **Build Directory**: All compiled binaries, libraries, and CMake files
- **Temporary Files**: CMake cache, compilation artifacts, log files
- **Result Files**: Benchmark results, test outputs, core dumps
- **Backup Files**: Editor backups, temporary saves, swap files

#### Common Usage:
```bash
# Before fresh build
./scripts/clean_build.sh all
mkdir build && cd build
cmake .. && make

# Clean only results between benchmark runs
./scripts/clean_build.sh results

# Quick cleanup of temporary files
./scripts/clean_build.sh temp
```

**Note**: The script provides safety checks and colorized output to show what's being cleaned. Combined with the project's [`.gitignore`](.gitignore) file, this ensures a clean development environment free of build artifacts and temporary files.

### Contributing

1. Follow C++17 standards
2. Use consistent naming conventions
3. Add appropriate error handling
4. Include comprehensive tests
5. Update documentation

### Version Control

The project includes a comprehensive [`.gitignore`](.gitignore) file that excludes:

#### Build Artifacts:
- `build/` directory and all contents
- Compiled executables (*.exe, *.out, *.app)
- Libraries (*.so, *.dll, *.a, *.lib)
- Object files (*.o, *.obj)
- CMake generated files (CMakeCache.txt, CMakeFiles/)

#### Development Files:
- IDE configurations (.vscode/, .idea/, *.vcxproj)
- Editor temporary files (*.swp, *~, .#*)
- Backup files (*.bak, *.orig)
- Log files (*.log, *.tmp)

#### Test and Benchmark Results:
- Benchmark results (*benchmark_results*.csv)
- Test outputs (*test_results*.txt)
- Performance logs (*output*.log)

#### System Files:
- OS-specific files (.DS_Store, Thumbs.db)
- Core dumps (core, vgcore.*)
- Debug files (*.pdb, *.dSYM/)

#### Project-Specific:
- PostgreSQL backups (*.conf.backup)
- CUDA generated files (*.cubin, *.ptx)
- Local configuration files (config.local.*)

#### Git Workflow:
```bash
# Initialize repository (if needed)
git init

# Add source files
git add src/ examples/ tests/ scripts/ CMakeLists.txt README.md manual.md

# Commit changes
git commit -m "Initial commit"

# Check ignored files
git check-ignore build/CMakeCache.txt  # Should show it's ignored
git status  # Should not show build artifacts
```

**Note**: The `.gitignore` ensures that build artifacts, temporary files, and system-specific files are not accidentally committed to version control.

---

## Appendix

### Example Connection Strings

**Local PostgreSQL:**
```
postgresql://pgvuser:pgvpass@localhost:5432/vectordb
```

**Remote PostgreSQL:**
```
postgresql://username:password@hostname:5432/database
```

**With SSL:**
```
postgresql://username:password@hostname:5432/database?sslmode=require
```

### Useful SQL Commands

**Check pgvector installation:**
```sql
SELECT * FROM pg_available_extensions WHERE name = 'vector';
```

**List vector tables:**
```sql
SELECT schemaname, tablename FROM pg_tables WHERE tablename LIKE '%vector%';
```

**Check index storage:**
```sql
SELECT schemaname, tablename FROM pg_tables WHERE tablename LIKE '%faiss%';
```

### Performance Benchmarks

Typical performance on modern hardware (Intel i7, 16GB RAM):

| Operation | Vectors | Dimension | Time |
|-----------|---------|-----------|------|
| Add Vectors | 10,000 | 128 | ~5ms |
| Search (k=10) | 10,000 | 128 | ~50μs |
| Save Index | 10,000 | 128 | ~100ms |
| Load Index | 10,000 | 128 | ~50ms |

*Results may vary based on hardware, data distribution, and index configuration.*