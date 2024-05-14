#include <stdio.h>

__global__ void cuda_hello(){ // isso é um kernel
    printf("Hello World from GPU!\n");
}

int main() {
    cuda_hello<<<1,1>>>();  // número de threads em x e y que vão executar o kernel
    return 0;
}