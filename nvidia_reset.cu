// For the CUDA runtime routines (prefixed with "cuda_")
#include <stdio.h>
#include <cuda_runtime.h>

/**
 * Reset the NVIDIA device
 *
 * cudaDeviceReset causes the driver to clean up all state. While not mandatory in normal operation, it is good practice.
 * It is also needed to ensure correct operation when the application is being profiled. Calling cudaDeviceReset causes
 * all profile data to be flushed before the application exits
 */
int main(void)
{
    cudaError_t err = cudaDeviceReset();	// Error code to check return values for CUDA calls

    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to deinitialize the device! error=%s\n", cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    return 0;
}
