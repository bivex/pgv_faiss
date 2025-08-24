cmake_minimum_required(VERSION 3.16)
project(PGVectorFAISS VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(WITH_GPU "Enable GPU support" ON)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build tests" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

if(WITH_GPU)
    enable_language(CUDA)
    find_package(CUDAToolkit REQUIRED)
endif()

include_directories(src/include)
include_directories(${LIBPQ_INCLUDE_DIRS})

add_subdirectory(src/lib)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()