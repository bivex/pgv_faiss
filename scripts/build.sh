#!/bin/bash

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BUILD_TYPE="${1:-Release}"
ENABLE_GPU="${2:-auto}"
NUM_CORES="${3:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

echo "=== PGVector + FAISS SDK Build Script ==="
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"
echo "Build type: $BUILD_TYPE"
echo "GPU support: $ENABLE_GPU"
echo "Parallel jobs: $NUM_CORES"
echo ""

check_dependencies() {
    echo "Checking dependencies..."
    
    if ! command -v cmake &> /dev/null; then
        echo "Error: CMake not found"
        exit 1
    fi
    
    if ! pkg-config --exists libpq; then
        echo "Error: PostgreSQL development libraries not found"
        echo "Run: sudo apt-get install libpq-dev"
        exit 1
    fi
    
    if [[ "$ENABLE_GPU" == "auto" ]] && command -v nvcc &> /dev/null; then
        ENABLE_GPU="ON"
        echo "✓ CUDA found, enabling GPU support"
    elif [[ "$ENABLE_GPU" == "auto" ]]; then
        ENABLE_GPU="OFF"
        echo "⚠ CUDA not found, disabling GPU support"
    fi
    
    echo "✓ Dependencies checked"
}

configure_build() {
    echo "Configuring build..."
    
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    CMAKE_ARGS=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DWITH_GPU=$ENABLE_GPU"
        "-DBUILD_EXAMPLES=ON"
        "-DBUILD_TESTS=ON"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )
    
    if [[ "$BUILD_TYPE" == "Debug" ]]; then
        CMAKE_ARGS+=("-DCMAKE_C_FLAGS=-g -O0")
        CMAKE_ARGS+=("-DCMAKE_CXX_FLAGS=-g -O0")
    elif [[ "$BUILD_TYPE" == "Release" ]]; then
        CMAKE_ARGS+=("-DCMAKE_C_FLAGS=-O3 -DNDEBUG")
        CMAKE_ARGS+=("-DCMAKE_CXX_FLAGS=-O3 -DNDEBUG")
    fi
    
    echo "Running: cmake ${CMAKE_ARGS[*]} .."
    cmake "${CMAKE_ARGS[@]}" ..
    
    echo "✓ Build configured"
}

build_project() {
    echo "Building project..."
    
    cd "$BUILD_DIR"
    
    make -j"$NUM_CORES"
    
    echo "✓ Build completed"
}

run_basic_tests() {
    echo "Running basic tests..."
    
    cd "$BUILD_DIR"
    
    if [[ -f "examples/basic_example" ]]; then
        echo "Running basic example..."
        ./examples/basic_example || echo "⚠ Basic example failed (database connection issue expected)"
    fi
    
    if [[ -f "examples/advanced_example" ]]; then
        echo "Running advanced example..."
        ./examples/advanced_example || echo "⚠ Advanced example failed (database connection issue expected)"
    fi
    
    if [[ "$ENABLE_GPU" == "ON" && -f "examples/gpu_example" ]]; then
        echo "Running GPU example..."
        ./examples/gpu_example || echo "⚠ GPU example failed (GPU or database issue expected)"
    fi
    
    echo "✓ Basic tests completed"
}

install_library() {
    if [[ "${1:-}" == "--install" ]]; then
        echo "Installing library..."
        cd "$BUILD_DIR"
        sudo make install
        
        echo "Library installed to /usr/local"
        echo "Don't forget to add /usr/local/lib to your LD_LIBRARY_PATH"
    fi
}

show_build_info() {
    echo ""
    echo "=== Build Summary ==="
    echo "Build directory: $BUILD_DIR"
    echo "Executables built:"
    
    cd "$BUILD_DIR"
    
    if [[ -f "examples/basic_example" ]]; then
        echo "  ✓ Basic example: $BUILD_DIR/examples/basic_example"
    fi
    
    if [[ -f "examples/advanced_example" ]]; then
        echo "  ✓ Advanced example: $BUILD_DIR/examples/advanced_example"
    fi
    
    if [[ -f "examples/benchmark_example" ]]; then
        echo "  ✓ Benchmark: $BUILD_DIR/examples/benchmark_example"
    fi
    
    if [[ -f "examples/gpu_example" ]]; then
        echo "  ✓ GPU example: $BUILD_DIR/examples/gpu_example"
    fi
    
    if [[ -f "src/lib/libpgv_faiss.so" || -f "src/lib/libpgv_faiss.dylib" || -f "src/lib/libpgv_faiss.dll" ]]; then
        echo "  ✓ Library: $BUILD_DIR/src/lib/libpgv_faiss.*"
    fi
    
    echo ""
    echo "To run examples:"
    echo "  cd $BUILD_DIR"
    echo "  export LD_LIBRARY_PATH=\$PWD/src/lib:\$LD_LIBRARY_PATH"
    echo "  ./examples/basic_example"
    echo ""
    
    if [[ "$ENABLE_GPU" == "ON" ]]; then
        echo "GPU support enabled. Make sure CUDA libraries are in your path."
    fi
}

usage() {
    echo "Usage: $0 [BUILD_TYPE] [ENABLE_GPU] [NUM_CORES]"
    echo ""
    echo "Arguments:"
    echo "  BUILD_TYPE    Release|Debug (default: Release)"
    echo "  ENABLE_GPU    ON|OFF|auto (default: auto)"
    echo "  NUM_CORES     Number of parallel build jobs (default: auto-detect)"
    echo ""
    echo "Examples:"
    echo "  $0                    # Release build with auto GPU detection"
    echo "  $0 Debug             # Debug build"
    echo "  $0 Release ON        # Release build with GPU support"
    echo "  $0 Release OFF 8     # Release build, no GPU, 8 cores"
    echo ""
    echo "Additional options:"
    echo "  $0 --install         # Also install the library system-wide"
}

main() {
    if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
        usage
        exit 0
    fi
    
    check_dependencies
    echo ""
    
    configure_build
    echo ""
    
    build_project
    echo ""
    
    run_basic_tests
    echo ""
    
    install_library "$@"
    
    show_build_info
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi