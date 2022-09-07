#pragma once
#include <stdexcept>
#include <string>
#include <iostream>
#include <memory>
#include <chrono>
#include <cstdlib>

#define CK_CUDA_THROW(cmd)                              \
    do {                                                \
        cudaError_t retval = (cmd);                     \
        if (retval != cudaSuccess) {                    \
            throw std::runtime_error(                   \
                std::string("CUDA Runtime error: ") +   \
                (cudaGetErrorString(retval)) + " " +    \
                __FILE__ + ":" +                        \
                std::to_string(__LINE__) + " \n");      \
        }                                               \
    } while (0)

#define BLOCKS_PER_GRID(N)                              \
    ((N + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK)

template<typename T>
std::shared_ptr<T> getInitData(int count) {
    srand(11);
    T *p = new T[count];
    for (int i = 0; i < count; ++i) {
        p[i] = T((rand() - RAND_MAX / 2) * 1.0 / (RAND_MAX / 2 / 0.5));
    }
    return std::shared_ptr<T>(p, [](T *p) { delete[] p; });
}

template<typename T>
std::shared_ptr<T> getDeviceArray(int count, T *h_ptr) {
    T *d_ptr = nullptr;
    CK_CUDA_THROW(cudaMalloc(&d_ptr, count*sizeof(T)));
    CK_CUDA_THROW(cudaMemcpy(d_ptr, h_ptr, count*sizeof(T), cudaMemcpyHostToDevice));
    return std::shared_ptr<T>(d_ptr, [](void *p) { CK_CUDA_THROW(cudaFree(p)); });
}

template<typename T>
void printDeviceArray(const T *d_ptr, int count, int row=INT_MAX) {
    T *h_ptr = new T[count];
    CK_CUDA_THROW(cudaMemcpy(h_ptr, d_ptr, count*sizeof(T), cudaMemcpyDeviceToHost));
    for (int i = 0; i < count; ++i) {
        std::cout << (float)h_ptr[i] << " ";
        if ((i + 1) % row == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    delete[] h_ptr;
}

class Timer {
public:
    void start(const std::string &name = "") {
        name_  = name;
        start_ = std::chrono::high_resolution_clock::now();
    }

    float stop(const int repeats = 1) {
        stop_   = std::chrono::high_resolution_clock::now();
        deltat_ = std::chrono::duration_cast<std::chrono::microseconds>(stop_ - start_);
        printf("%s time cost: %10.4f ms/run\n\n", name_.c_str(), deltat_.count() / 1000.0 / repeats);
        return deltat_.count() / 1000.0 / repeats;
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
    std::chrono::high_resolution_clock::time_point stop_;
    std::chrono::microseconds deltat_;
    std::string name_;
};
