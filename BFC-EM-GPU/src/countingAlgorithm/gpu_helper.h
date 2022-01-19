#ifndef	GPU_ERROR_CHECK_CUH
#define	GPU_ERROR_CHECK_CUH

#include <string>
#include <sstream>
#include <stdexcept>
#include <sys/time.h>
#include <stdlib.h>

#define GEC(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

#define ARRAY_SIZE_IN_BYTES(n) (1LL* sizeof(unsigned int) * (n))
#define ARRAY_SIZE_IN_BYTES_LL(n) (1LL * sizeof(unsigned long long) * (n))
#define GMallocInt(ptr, size) cudaMalloc((void**) &ptr, ARRAY_SIZE_IN_BYTES((size)))
#define GMallocLL(ptr, size) cudaMalloc((void**) &ptr, ARRAY_SIZE_IN_BYTES_LL((size)))
#endif	//	GPU_ERROR_CHECK_CUH