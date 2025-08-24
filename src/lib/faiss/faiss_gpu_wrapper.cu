#ifdef WITH_GPU

#include "faiss_wrapper.h"
#include <faiss/gpu/GpuIndexFlat.h>
#include <faiss/gpu/GpuIndexIVFFlat.h>
#include <faiss/gpu/StandardGpuResources.h>
#include <faiss/gpu/utils/DeviceUtils.h>
#include <cuda_runtime.h>
#include <iostream>

class FAISSWrapper::GPUImpl {
public:
    std::unique_ptr<faiss::gpu::StandardGpuResources> gpu_resources;
    int gpu_device;
    
    GPUImpl(int device) : gpu_device(device) {
        setup_gpu_resources();
    }
    
    void setup_gpu_resources() {
        // TODO: Add GPU capability checking (compute capability, memory size)
        // TODO: Implement GPU memory optimization strategies
        // TODO: Add support for multiple GPU devices and load balancing
        // TODO: Implement GPU memory monitoring and adaptive allocation
        
        if (faiss::gpu::getNumDevices() <= gpu_device) {
            std::cerr << "GPU device " << gpu_device << " not available" << std::endl;
            return;
        }
        
        cudaError_t err = cudaSetDevice(gpu_device);
        if (err != cudaSuccess) {
            std::cerr << "Failed to set CUDA device " << gpu_device 
                      << ": " << cudaGetErrorString(err) << std::endl;
            return;
        }
        
        try {
            gpu_resources = std::make_unique<faiss::gpu::StandardGpuResources>();
            
            size_t free_mem, total_mem;
            cudaMemGetInfo(&free_mem, &total_mem);
            
            // TODO: Make memory allocation strategy configurable
            // TODO: Add memory pool management for better performance
            size_t temp_mem = std::min(free_mem / 4, size_t(1536 * 1024 * 1024));
            gpu_resources->setTempMemory(temp_mem);
            
            std::cout << "GPU " << gpu_device << " initialized with " 
                      << temp_mem / (1024*1024) << " MB temp memory" << std::endl;
                      
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize GPU resources: " << e.what() << std::endl;
            gpu_resources.reset();
        }
    }
    
    faiss::Index* create_gpu_index(const std::string& index_type, int dimension) {
        if (!gpu_resources) {
            return nullptr;
        }
        
        // TODO: Add automatic GPU index selection based on data size and GPU memory
        // TODO: Implement fallback to CPU when GPU memory is insufficient
        // TODO: Add support for more GPU index types (GpuIndexIVFPQ, etc.)
        // TODO: Implement index parameter optimization based on GPU architecture
        
        try {
            if (index_type == "Flat") {
                return new faiss::gpu::GpuIndexFlat(
                    gpu_resources.get(), dimension, faiss::METRIC_L2);
            }
            else if (index_type == "IVFFlat") {
                // TODO: Make ncentroids adaptive based on dataset size and GPU memory
                int ncentroids = std::min(4 * (int)sqrt(100000), 65536);
                return new faiss::gpu::GpuIndexIVFFlat(
                    gpu_resources.get(), dimension, ncentroids, faiss::METRIC_L2);
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to create GPU index: " << e.what() << std::endl;
        }
        
        return nullptr;
    }
    
    void print_gpu_info() {
        int device_count;
        cudaGetDeviceCount(&device_count);
        
        std::cout << "Found " << device_count << " CUDA devices:" << std::endl;
        
        for (int i = 0; i < device_count; ++i) {
            cudaDeviceProp prop;
            cudaGetDeviceProperties(&prop, i);
            
            std::cout << "  Device " << i << ": " << prop.name 
                      << " (" << prop.major << "." << prop.minor << ")" << std::endl;
            std::cout << "    Memory: " << prop.totalGlobalMem / (1024*1024) << " MB" << std::endl;
            std::cout << "    Multiprocessors: " << prop.multiProcessorCount << std::endl;
        }
    }
};

void FAISSWrapper::setup_gpu_resources() {
    if (use_gpu_) {
        gpu_impl_ = std::make_unique<GPUImpl>(gpu_device_);
    }
}

faiss::Index* FAISSWrapper::create_gpu_index(const std::string& index_type, int dimension) {
    if (use_gpu_ && gpu_impl_) {
        return gpu_impl_->create_gpu_index(index_type, dimension);
    }
    return nullptr;
}

void FAISSWrapper::print_gpu_info() {
    if (gpu_impl_) {
        gpu_impl_->print_gpu_info();
    }
}

__global__ void warm_up_gpu() {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    volatile float dummy = sinf(float(idx));
}

void FAISSWrapper::warm_up_gpu() {
    if (use_gpu_) {
        cudaSetDevice(gpu_device_);
        warm_up_gpu<<<256, 256>>>();
        cudaDeviceSynchronize();
    }
}

#endif