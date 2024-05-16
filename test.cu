#include <stdio.h>
#include <stdlib.h>
#define N 10000000
// block --> 256 threads 
// CUDA kernel to add elements of two arrays
__global__ void vector_add(float *out, const float *a, const float *b, int n) {
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    if (index < n) {
        out[index] = a[index] + b[index];
    }
}

int main() {
    float *a, *b, *out;
    float *d_a, *d_b, *d_out;

    // Allocate host memory
    a = (float*)malloc(sizeof(float) * N);
    b = (float*)malloc(sizeof(float) * N);
    out = (float*)malloc(sizeof(float) * N);

    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = 1.0f; // Example values
        b[i] = 2.0f; // Example values
    }

    // Allocate device memory
    cudaMalloc((void**)&d_a, sizeof(float) * N);
    cudaMalloc((void**)&d_b, sizeof(float) * N);
    cudaMalloc((void**)&d_out, sizeof(float) * N);

    // Transfer data from host to device memory
    cudaMemcpy(d_a, a, sizeof(float) * N, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, sizeof(float) * N, cudaMemcpyHostToDevice);

    // Execute the kernel
    int blockSize = 256; // Number of threads per block
    int numBlocks = (N + blockSize - 1) / blockSize; // Number of blocks
    vector_add<<<numBlocks, blockSize>>>(d_out, d_a, d_b, N);

    // Copy the results back to the host
    cudaMemcpy(out, d_out, sizeof(float) * N, cudaMemcpyDeviceToHost);

    // Print a few results
    for (int i = 0; i < 100; i++) {
        printf("%f ", out[i]);
    }
    printf("\n");

    // Cleanup
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_out);
    free(a);
    free(b);
    free(out);

    return 0;
}
