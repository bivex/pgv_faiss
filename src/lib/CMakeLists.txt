set(PGV_FAISS_SOURCES
    core/pgv_faiss_core.cpp
    pgvector/pgv_connection.cpp
    pgvector/pgv_operations.cpp
)

# Try to find FAISS (optional)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(FAISS QUIET faiss)
endif()

if(FAISS_FOUND)
    message(STATUS "FAISS found, enabling FAISS support")
    list(APPEND PGV_FAISS_SOURCES faiss/faiss_wrapper.cpp)
    set(HAVE_FAISS 1)
else()
    message(STATUS "FAISS not found, using stub implementation")
    list(APPEND PGV_FAISS_SOURCES faiss/faiss_stub.cpp)
    set(HAVE_FAISS 0)
endif()

if(WITH_GPU AND FAISS_FOUND)
    list(APPEND PGV_FAISS_SOURCES faiss/faiss_gpu_wrapper.cu)
endif()

add_library(pgv_faiss ${PGV_FAISS_SOURCES})

target_include_directories(pgv_faiss PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${LIBPQ_INCLUDE_DIRS}
)

target_link_libraries(pgv_faiss 
    ${LIBPQ_LIBRARIES}
)

if(FAISS_FOUND)
    target_link_libraries(pgv_faiss ${FAISS_LIBRARIES})
    target_include_directories(pgv_faiss PRIVATE ${FAISS_INCLUDE_DIRS})
endif()

if(WITH_GPU AND FAISS_FOUND)
    target_link_libraries(pgv_faiss 
        CUDA::cudart
        CUDA::cublas
    )
    set_property(TARGET pgv_faiss PROPERTY CUDA_ARCHITECTURES 70 75 80 86)
endif()

target_compile_definitions(pgv_faiss PRIVATE 
    $<$<BOOL:${WITH_GPU}>:WITH_GPU>
    HAVE_FAISS=${HAVE_FAISS}
)
