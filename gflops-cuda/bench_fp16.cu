#include "common.h"

#include "cuda_fp16.h"

#define LOOP 10000

__global__
void fmla(half2 *in, int count) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < count) {
        half2 val = in[idx];
        for (int i = 0; i < LOOP; ++i) {
            val += val * val;
            val += val * val;
            val += val * val;
            val += val * val;

            val += val * val;
            val += val * val;
            val += val * val;
            val += val * val;

            val += val * val;
            val += val * val;
            val += val * val;
            val += val * val;

            val += val * val;
            val += val * val;
            val += val * val;
            val += val * val;
        }
        in[idx] = val;
    }
}

int main() {
    int count = 1<<20;
    auto h_in = getInitData<half>(count);
    auto in = getDeviceArray(count, h_in.get());
    printDeviceArray(in.get(), 10);

    int THREADS_PER_BLOCK = 256;
    fmla<<<BLOCKS_PER_GRID(count / 2), THREADS_PER_BLOCK>>>((half2*)in.get(), count);
    printDeviceArray(in.get(), 10);

    {
        int wc = 5;
        for (int i = 0; i < wc; ++i) {
            fmla<<<BLOCKS_PER_GRID(count / 2), THREADS_PER_BLOCK>>>((half2*)in.get(), count);
        }
        CK_CUDA_THROW(cudaDeviceSynchronize());

        Timer timer;
        timer.start("fmla");
        int ic = 10;
        for (int i = 0; i < ic; ++i) {
            fmla<<<BLOCKS_PER_GRID(count / 2), THREADS_PER_BLOCK>>>((half2*)in.get(), count);
        }
        CK_CUDA_THROW(cudaDeviceSynchronize());
        float ms = timer.stop(ic);
        printf("GFlops = %f\n", 1.0 * count * 16 * 2 * LOOP / ms / 1000000.0);
    }

    return 0;
}
