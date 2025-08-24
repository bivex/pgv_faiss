#!/bin/bash

set -e

echo "=== PGVector + FAISS SDK Dependency Installation ==="

INSTALL_PREFIX="/usr/local"
FAISS_VERSION="v1.7.4"
CUDA_ENABLED=${1:-"auto"}

check_cuda() {
    if command -v nvcc &> /dev/null; then
        echo "✓ CUDA detected: $(nvcc --version | grep release)"
        return 0
    else
        echo "⚠ CUDA not found"
        return 1
    fi
}

install_system_deps() {
    echo "Installing system dependencies..."
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            git \
            pkg-config \
            libpq-dev \
            postgresql-client \
            python3-dev \
            python3-pip \
            libblas-dev \
            liblapack-dev \
            libopenblas-dev
        
        if [[ "$CUDA_ENABLED" != "no" ]] && check_cuda; then
            sudo apt-get install -y \
                nvidia-cuda-toolkit \
                libcublas-dev \
                libcurand-dev \
                libcusparse-dev
        fi
        
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        if ! command -v brew &> /dev/null; then
            echo "Error: Homebrew not found. Please install Homebrew first."
            exit 1
        fi
        
        brew install \
            cmake \
            pkg-config \
            postgresql \
            openblas \
            lapack
            
    elif [[ "$OSTYPE" == "msys" ]]; then
        echo "Windows detected. Please ensure you have:"
        echo "- Visual Studio 2019 or later"
        echo "- CMake 3.16+"
        echo "- vcpkg or manually installed dependencies"
        echo "- PostgreSQL development libraries"
    fi
    
    echo "✓ System dependencies installed"
}

install_faiss() {
    echo "Installing FAISS..."
    
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    git clone --depth 1 --branch "$FAISS_VERSION" https://github.com/facebookresearch/faiss.git
    cd faiss
    
    mkdir -p build
    cd build
    
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX -DFAISS_ENABLE_PYTHON=OFF"
    
    if [[ "$CUDA_ENABLED" != "no" ]] && check_cuda; then
        CMAKE_ARGS="$CMAKE_ARGS -DFAISS_ENABLE_GPU=ON"
        echo "Building FAISS with GPU support..."
    else
        CMAKE_ARGS="$CMAKE_ARGS -DFAISS_ENABLE_GPU=OFF"
        echo "Building FAISS without GPU support..."
    fi
    
    cmake .. $CMAKE_ARGS
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    sudo make install
    
    cd /
    rm -rf "$TEMP_DIR"
    
    echo "✓ FAISS installed"
}

setup_postgresql() {
    echo "Setting up PostgreSQL with pgvector..."
    
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    git clone --depth 1 --branch v0.5.0 https://github.com/pgvector/pgvector.git
    cd pgvector
    
    make
    sudo make install
    
    cd /
    rm -rf "$TEMP_DIR"
    
    echo "✓ pgvector extension installed"
    echo ""
    echo "To complete pgvector setup, run in your PostgreSQL database:"
    echo "  CREATE EXTENSION vector;"
}

create_test_db() {
    echo "Creating test database..."
    
    if command -v psql &> /dev/null; then
        createdb vectortest 2>/dev/null || true
        psql -d vectortest -c "CREATE EXTENSION IF NOT EXISTS vector;" 2>/dev/null || echo "⚠ Could not create test database"
        echo "✓ Test database setup attempted"
    else
        echo "⚠ PostgreSQL client not found. Please create test database manually."
    fi
}

main() {
    echo "Installation prefix: $INSTALL_PREFIX"
    echo "CUDA enabled: $CUDA_ENABLED"
    echo ""
    
    install_system_deps
    echo ""
    
    install_faiss
    echo ""
    
    setup_postgresql
    echo ""
    
    create_test_db
    echo ""
    
    echo "=== Installation Complete ==="
    echo ""
    echo "Next steps:"
    echo "1. Set environment variables:"
    echo "   export PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig:\$PKG_CONFIG_PATH"
    echo "   export LD_LIBRARY_PATH=$INSTALL_PREFIX/lib:\$LD_LIBRARY_PATH"
    echo ""
    echo "2. Build the project:"
    echo "   mkdir build && cd build"
    echo "   cmake .."
    echo "   make -j\$(nproc)"
    echo ""
    echo "3. Run tests:"
    echo "   ./examples/basic_example"
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi