# pgv_faiss Tools

This directory contains utility tools for managing and maintaining the pgv_faiss system.

## Database Cleanup Tool (db_cleanup.cpp)

The database cleanup tool is designed to clear vector tables and FAISS indices from the PostgreSQL database, making it ideal for:

- **Benchmarking**: Clear the database before running performance tests
- **Development**: Reset the database state during development
- **Testing**: Ensure clean test environments
- **Maintenance**: Remove old or test data

### Features

- **Selective Cleanup**: Choose what to clean (vectors, indices, or both)
- **Safety**: Only targets tables with vector-related naming patterns
- **Statistics**: Show database size and table information
- **Vacuum**: Optional database vacuum operation for space reclamation
- **Interactive**: Command-line interface with helpful output

### Usage

#### Build the tool:
```bash
cd build
make db_cleanup
```

#### Run cleanup operations:
```bash
# Set library path
export LD_LIBRARY_PATH=/path/to/pgv_faiss/build/src/lib:$LD_LIBRARY_PATH

# Complete cleanup
./examples/db_cleanup --all

# Clear only vector tables
./examples/db_cleanup --vectors

# Clear only FAISS indices
./examples/db_cleanup --indices

# Show database statistics
./examples/db_cleanup --stats

# Full cleanup with vacuum
./examples/db_cleanup --all --vacuum

# Show help
./examples/db_cleanup --help
```

#### Alternative: Use the convenience script:
```bash
# Make script executable (first time only)
chmod +x scripts/cleanup_db.sh

# Run operations
./scripts/cleanup_db.sh all      # Complete cleanup
./scripts/cleanup_db.sh vectors  # Clear vector tables
./scripts/cleanup_db.sh indices  # Clear FAISS indices
./scripts/cleanup_db.sh stats    # Show statistics
./scripts/cleanup_db.sh vacuum   # Full cleanup + vacuum
```

### What Gets Cleaned

The cleanup tool identifies and removes tables with these naming patterns:

- **Vector Tables**: Tables containing "test", "vector", "embedding", "sample"
- **FAISS Indices**: Tables ending with "_faiss_index"
- **Benchmark Data**: Tables containing "benchmark", "index"

### Safety Features

- **Pattern Matching**: Only removes tables matching specific patterns
- **System Table Protection**: Avoids PostgreSQL system tables
- **Confirmation Output**: Shows exactly what tables will be removed
- **Error Handling**: Graceful error handling with informative messages

### Database Connection

The tool uses the standard pgv_faiss database configuration:
- **Host**: localhost
- **Port**: 5432
- **Database**: vectordb
- **Username**: pgvuser
- **Password**: pgvpass

To use a different database, modify the `connection_string` variable in the source code.

### Examples

#### Before Running Benchmarks:
```bash
# Clear everything and vacuum for optimal performance
./scripts/cleanup_db.sh vacuum
```

#### Development Workflow:
```bash
# Check current database state
./scripts/cleanup_db.sh stats

# Clear test data
./scripts/cleanup_db.sh vectors

# Check results
./scripts/cleanup_db.sh stats
```

#### Maintenance:
```bash
# Clear old indices but keep vector data
./scripts/cleanup_db.sh indices
```

### Building From Source

The cleanup tool is automatically built when building the examples:

```bash
cd pgv_faiss
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
make db_cleanup
```

### Dependencies

- PostgreSQL with pgvector extension
- libpq (PostgreSQL client library)
- Standard C++ libraries (C++17)

### Error Handling

The tool provides clear error messages for common issues:

- Database connection failures
- Permission denied errors
- Missing tables or indices
- SQL execution errors

Check the output for specific error messages and refer to the main manual for troubleshooting guidance.

---

*This tool is part of the pgv_faiss project. For more information, see the main manual.md file.*