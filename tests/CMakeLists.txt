find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

# Unit tests
add_executable(database_test unit/database_test.cpp)
target_link_libraries(database_test pgv_faiss ${LIBPQ_LIBRARIES})
target_include_directories(database_test PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

add_executable(simple_test unit/simple_test.cpp)
target_link_libraries(simple_test pgv_faiss ${LIBPQ_LIBRARIES})
target_include_directories(simple_test PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

# Test target to run all tests
add_custom_target(run_tests
    COMMAND echo "Running pgv_faiss unit tests..."
    COMMAND echo "=== Simple Library Test ==="
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/simple_test
    COMMAND echo "=== Database Connection Test ==="
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/database_test || echo "Database test failed (expected if no database running)"
    DEPENDS simple_test database_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Add to main test target if CTest is enabled
if(BUILD_TESTING)
    enable_testing()
    add_test(NAME simple_test COMMAND simple_test)
    add_test(NAME database_test COMMAND database_test)
endif()