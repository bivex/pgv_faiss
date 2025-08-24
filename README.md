# PGVector + FAISS SDK

A high-performance C/C++ SDK that combines PostgreSQL's pgvector extension with Facebook's FAISS library for scalable vector similarity search.

## Features

- **Hybrid Architecture**: Combines pgvector's persistence with FAISS's high-performance indexing
- **GPU Acceleration**: Optional CUDA support for massive performance gains
- **Multiple Index Types**: Support for Flat, IVF, and HNSW indices
- **Production Ready**: Thread-safe, memory-efficient, and battle-tested
- **Easy Integration**: Simple C API with comprehensive examples

## Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libpq-dev postgresql-client

# macOS
brew install cmake postgresql

# Install dependencies automatically
./scripts/install_dependencies.sh
```

### Build

```bash
# Simple build
./scripts/build.sh

# Custom build options
./scripts/build.sh Release ON 8  # Release build with GPU support, 8 cores
```

### Basic Usage

```c
#include "pgv_faiss.h"

// Configure the SDK
pgv_faiss_config_t config = {
    .connection_string = "postgresql://user:pass@localhost/db",
    .dimension = 512,
    .use_gpu = 1,
    .index_type = "IVFFlat"
};

// Initialize index
pgv_faiss_index_t* index;
pgv_faiss_init(&config, &index);

// Add vectors
float vectors[1000 * 512];  // Your vector data
int64_t ids[1000];          // Vector IDs
pgv_faiss_add_vectors(index, vectors, ids, 1000);

// Search
pgv_faiss_result_t result;
float query[512];           // Query vector
pgv_faiss_search(index, query, 10, &result);

// Use results
for (size_t i = 0; i < result.count; i++) {
    printf("ID: %ld, Distance: %f\n", result.ids[i], result.distances[i]);
}

// Cleanup
pgv_faiss_free_result(&result);
pgv_faiss_destroy(index);
```

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Application   │    │   PGV-FAISS     │    │   PostgreSQL    │
│      Code       │◄──►│      SDK        │◄──►│   + pgvector    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                       ┌─────────────────┐
                       │  FAISS Library  │
                       │   (CPU/GPU)     │
                       └─────────────────┘
```

## Project Structure

```
pgv_faiss/
├── src/
│   ├── include/
│   │   └── pgv_faiss.h          # Main SDK header
│   └── lib/
│       ├── core/                # Core SDK implementation
│       ├── pgvector/            # PostgreSQL integration
│       └── faiss/               # FAISS wrapper
├── examples/
│   ├── basic/                   # Simple usage examples
│   ├── advanced/                # Advanced features demo
│   └── benchmarks/              # Performance benchmarks
├── scripts/
│   ├── install_dependencies.sh  # Dependency installation
│   └── build.sh                 # Build automation
└── docs/                        # Documentation
```

## API Reference

### Configuration

```c
typedef struct pgv_faiss_config {
    char* connection_string;  // PostgreSQL connection
    int dimension;           // Vector dimension
    int use_gpu;            // Enable GPU acceleration (0/1)
    int gpu_device_id;      // GPU device ID
    char* index_type;       // "Flat", "IVFFlat", "HNSW"
    int nprobe;            // Search parameter for IVF indices
} pgv_faiss_config_t;
```

### Core Functions

| Function | Description |
|----------|-------------|
| `pgv_faiss_init()` | Initialize index with configuration |
| `pgv_faiss_add_vectors()` | Add vectors to the index |
| `pgv_faiss_search()` | Perform similarity search |
| `pgv_faiss_save_to_db()` | Persist index to PostgreSQL |
| `pgv_faiss_load_from_db()` | Load index from PostgreSQL |
| `pgv_faiss_destroy()` | Clean up resources |

## Index Types

| Type | Use Case | Performance | Memory |
|------|----------|-------------|---------|
| **Flat** | Small datasets, exact search | Slow for large data | High |
| **IVFFlat** | Medium to large datasets | Fast | Medium |
| **HNSW** | Large datasets, approximate search | Very fast | High |

## GPU Acceleration

Enable GPU support for 10-100x performance improvements:

```c
pgv_faiss_config_t config = {
    .use_gpu = 1,
    .gpu_device_id = 0,  // Use first GPU
    // ... other config
};
```

**Requirements:**
- CUDA Toolkit 11.0+
- CUDA-capable GPU with compute capability 7.0+
- Sufficient GPU memory for your dataset

## Examples

### Basic Example
```bash
cd build
./examples/basic_example
```

### Advanced Features
```bash
./examples/advanced_example
```

### GPU Acceleration
```bash
./examples/gpu_example
```

### Performance Benchmarks
```bash
./examples/benchmark_example
```

## Performance Tips

1. **Choose the Right Index**: Use IVFFlat for most cases, HNSW for read-heavy workloads
2. **GPU Memory**: Ensure your GPU has enough memory (vectors × dimension × 4 bytes)
3. **Batch Operations**: Add vectors in batches for better performance
4. **Connection Pooling**: Use connection pooling for high-throughput applications

## Troubleshooting

### Common Issues

**Build fails with missing PostgreSQL**
```bash
sudo apt-get install libpq-dev postgresql-server-dev-all
```

**FAISS not found**
```bash
./scripts/install_dependencies.sh
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

**GPU initialization fails**
```bash
# Check CUDA installation
nvcc --version
nvidia-smi

# Verify GPU memory
./examples/gpu_example
```

**Database connection issues**
```bash
# Create test database
createdb vectortest
psql vectortest -c "CREATE EXTENSION vector;"
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Run benchmarks to ensure performance
5. Submit a pull request

## License

MIT License - see LICENSE file for details.

## Acknowledgments

- [pgvector](https://github.com/pgvector/pgvector) - PostgreSQL vector extension
- [FAISS](https://github.com/facebookresearch/faiss) - Facebook AI Similarity Search
- PostgreSQL development team