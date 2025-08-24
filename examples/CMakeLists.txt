find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

add_executable(basic_example basic/basic_usage.cpp)
target_link_libraries(basic_example pgv_faiss ${LIBPQ_LIBRARIES})
target_include_directories(basic_example PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

add_executable(advanced_example advanced/advanced_usage.cpp)
target_link_libraries(advanced_example pgv_faiss ${LIBPQ_LIBRARIES})
target_include_directories(advanced_example PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

add_executable(benchmark_example benchmarks/benchmark.cpp)
target_link_libraries(benchmark_example pgv_faiss ${LIBPQ_LIBRARIES})
target_include_directories(benchmark_example PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

add_executable(db_cleanup tools/db_cleanup.cpp)
target_link_libraries(db_cleanup ${LIBPQ_LIBRARIES})
target_include_directories(db_cleanup PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

# GPU example - builds with or without CUDA
add_executable(gpu_example advanced/gpu_example.cpp)
target_link_libraries(gpu_example pgv_faiss ${LIBPQ_LIBRARIES})
target_include_directories(gpu_example PRIVATE ${CMAKE_SOURCE_DIR}/src/include)

if(WITH_GPU)
    # Add CUDA libraries if available
    target_link_libraries(gpu_example CUDA::cudart)
    target_compile_definitions(gpu_example PRIVATE WITH_GPU=1)
else()
    # Define macro to show it's compiled without GPU support
    target_compile_definitions(gpu_example PRIVATE WITHOUT_GPU=1)
endif()