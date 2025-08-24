# pgv_faiss Tests

This directory contains unit tests and integration tests for the pgv_faiss library.

## Test Structure

### Unit Tests (`unit/`)
Contains individual unit tests for library components:

- **[database_test.cpp](unit/database_test.cpp)** - Tests PostgreSQL database connection and basic initialization
- **[simple_test.cpp](unit/simple_test.cpp)** - Tests library loading, API accessibility, and error handling

## Building and Running Tests

### Build Tests
```bash
cd build
cmake .. -DBUILD_TESTS=ON
make
```

### Run Individual Tests
```bash
# Set library path
export LD_LIBRARY_PATH=src/lib:$LD_LIBRARY_PATH

# Run simple library test (no database required)
./tests/simple_test

# Run database connection test (requires PostgreSQL)
./tests/database_test
```

### Run All Tests
```bash
# Using CMake target
make run_tests

# Using CTest (if enabled)
ctest
```

## Test Descriptions

### Simple Test
- **Purpose**: Verify library loads correctly and API functions are accessible
- **Requirements**: None (no database needed)
- **Expected**: All tests pass, demonstrates error handling for missing database

### Database Test  
- **Purpose**: Test actual database connectivity and library initialization
- **Requirements**: PostgreSQL with pgvector extension, vectordb database
- **Expected**: Successful connection and initialization with proper database setup

## Test Configuration

### Database Connection
Tests use the standard pgv_faiss database configuration:
- **Connection String**: `postgresql://pgvuser:pgvpass@localhost:5432/vectordb`
- **Database**: vectordb
- **User**: pgvuser  
- **Password**: pgvpass

### Expected Behavior
- **Simple Test**: Always passes (tests library without database)
- **Database Test**: Passes with database, fails gracefully without
- **Error Codes**: -2 for database connection failure (expected without database)

## Adding New Tests

### Unit Test Template
```cpp
#include "pgv_faiss.h"
#include <iostream>

int main() {
    std::cout << "=== Your Test Name ===" << std::endl;
    
    // Your test code here
    
    std::cout << "✅ Test completed successfully!" << std::endl;
    return 0;
}
```

### Integration Steps
1. Add `.cpp` file to `unit/` directory
2. Update `CMakeLists.txt` with new executable
3. Add to `run_tests` target
4. Document in this README

## Troubleshooting

### Common Issues

#### Library Not Found
```
Error: libpgv_faiss.so: cannot open shared object file
```
**Solution**: Set LD_LIBRARY_PATH before running tests

#### Database Connection Failed
```
Database connection failed with code: -2
```
**Solution**: This is expected behavior when PostgreSQL is not running

#### Compilation Errors
```
Error: pgv_faiss.h: No such file or directory
```
**Solution**: Ensure you're building from the build directory with proper CMake configuration

## Test Results

### Expected Output

#### Simple Test (Success):
```
=== PGV-FAISS Simple Library Test ===
✓ Configuration structure created successfully
Testing pgv_faiss_init function...
✓ pgv_faiss_init correctly detected database connection failure (expected)
✓ pgv_faiss_result_t structure created successfully
✓ pgv_faiss_free_result handled null structure gracefully
✓ pgv_faiss_destroy handled null pointer gracefully

=== Library Test Summary ===
✓ Library loads correctly
✓ All API functions are accessible
✓ Error handling works as expected
✓ No crashes or undefined behavior detected

=== Test completed successfully ===
Note: Database connection tests failed as expected (no PostgreSQL server running)
```

#### Database Test (With Database):
```
=== Database Connection Test ===
Testing database connection...
✅ Database connection successful!
✅ pgv_faiss library initialized properly!
✅ Cleanup completed
```

#### Database Test (Without Database):
```
=== Database Connection Test ===
Testing database connection...
❌ Database connection failed with code: -2
```

---

*These tests are part of the pgv_faiss project test suite. For more information, see the main [manual.md](../manual.md) file.*