#ifndef _BARRIER_H_
#define _BARRIER_H_



#include "util.h"

__device__ __forceinline__ int ThreadLoad(int *ptr)
{
	int retval;           
	asm volatile ("ld.global.cg.s32 %0, [%1];" :    \
			"=r"(retval) :                        \
			"l" (ptr) );                          \
		return retval;  
}


class Barrier {
	
	public:
		int *d_lock;

		Barrier(int blks_num){
			setup(blks_num);	
		}

		virtual ~Barrier(){

		}
	public:

		void setup(int blks_num)
		{
			H_ERR(cudaMalloc((void **)&d_lock, sizeof(int)*blks_num));
			H_ERR(cudaMemset((void *)d_lock, 0, sizeof(int)*blks_num));
			H_ERR(cudaDeviceSynchronize());
		}


		//__device__ __forceinline__ void sync_grid(int 0, int 1)
		__device__ __forceinline__ void sync_grid_opt()
		{
			volatile int *lock = d_lock;	

			// Threadfence and syncthreads ensure global writes 
			// thread-0 reports in with its sync counter
			__threadfence();
			__syncthreads();

			if (blockIdx.x == 0)
			{
				// Report in ourselves
				if (threadIdx.x == 0)
					lock[blockIdx.x] = 1;

				__syncthreads();

				// Wait for everyone else to report in
				for (int peer_block = threadIdx.x; 
						peer_block < gridDim.x; peer_block += blockDim.x)
					while (ThreadLoad(d_lock + peer_block) == 0)
						__threadfence_block();
				
				__syncthreads();

				// Let everyone know it's safe to proceed
				for (int peer_block = threadIdx.x; 
                        peer_block < gridDim.x; peer_block += blockDim.x)
					lock[peer_block] = 0;
			}
			else
			{
				if (threadIdx.x == 0)
				{
					// Report in
					lock[blockIdx.x] = 1;

					// Wait for acknowledgment
					while (ThreadLoad(d_lock + blockIdx.x) != 0)
						__threadfence_block();
				}

				__syncthreads();
			}
		}
};

#endif
