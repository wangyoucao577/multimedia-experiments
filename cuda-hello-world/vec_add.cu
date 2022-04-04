
#include <stdio.h>

// kernel function that will be executed on GPU
__global__ void add(int *a, int *b, int *c) {
  int index = threadIdx.x + blockIdx.x * blockDim.x;
  c[index] = a[index] + b[index];
}

int main() {

  const int kThreadsPerBlock = 512;
  const int kN = 2048 * 2048;
  int *a, *b, *c;       // host copies of a, b, c
  int *d_a, *d_b, *d_c; // device copies of a, b, c
  int size = kN * sizeof(int);

  // allocate space for host copies of a, b, c
  a = (int *)malloc(size);
  b = (int *)malloc(size);
  c = (int *)malloc(size);

  // set input
  for (int i = 0; i < kN; i++) {
    a[i] = i;
    b[i] = 2 * i;
  }

  // allocate space for device copies of a, b, c
  cudaMalloc((void **)&d_a, size);
  cudaMalloc((void **)&d_b, size);
  cudaMalloc((void **)&d_c, size);

  // Copy inputs to device
  cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

  // Launch add() on GPU
  add<<<kN / kThreadsPerBlock, kThreadsPerBlock>>>(d_a, d_b, d_c);

  // Copy back to host
  auto err = cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);
  if (err != cudaSuccess) {
    printf("cudaMemcpy failed, err %s(%d)\n", cudaGetErrorString(err), err);
  }

  // print
  printf("Hello world, Vector add!\n");
  for (int i = 0; i < kN; i++) {
    if (i == 0 || i == kN - 1) {
      // only print first and last one
      printf("%d + %d = %d\n", a[i], b[i], c[i]);
    }
  }

  // Free
  cudaFree(d_a);
  cudaFree(d_b);
  cudaFree(d_c);
  free(a);
  free(b);
  free(c);

  return 0;
}
