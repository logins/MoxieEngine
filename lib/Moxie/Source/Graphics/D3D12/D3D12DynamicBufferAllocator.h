/*
 D3D12BufferAllocator.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef D3D12BufferAllocator_h__
#define D3D12BufferAllocator_h__

#include <deque>
#include "d3dx12.h"


namespace Mox{ 

	struct D3D12Resource;
	struct BufferResource;
	struct D3D12BufferResource;

	/*
	D3D12LinearBufferAllocator performs constant buffer sub-allocations in a single buffer resource.
	These sub-allocations are copied over a new memory location upon each frame, 
	giving the ability to the buffer to change value very often.
	It is meant to store small buffers that frequently change their value.
	The allocation system operates in a ring buffer fashon, similar to what is described here:
	https://www.codeproject.com/Articles/1094799/Implementing-Dynamic-Resources-with-Direct-D
	*/
	class D3D12DynamicBufferAllocator {

	public:
		D3D12DynamicBufferAllocator(Mox::D3D12Resource& InResource);

		~D3D12DynamicBufferAllocator();

		// Note: We do not need to pass the alignment since it is decided by the hosting resource
		Mox::BufferResource& Allocate(uint32_t InSize);



		// When frame starts, update the relative offset and copy in all the current active dynamic buffers 
		void OnFrameStarted();

		// When a frame ends, remove the oldest recorded offset
		void OnFrameEnded() { m_RelativeFrameOffsetStarts.pop_front(); }

		void Reset();

		// Do not allow copy construct
		D3D12DynamicBufferAllocator(const D3D12DynamicBufferAllocator& ) = delete;
		// Do not allow copy assignment
		D3D12DynamicBufferAllocator& operator=(const D3D12DynamicBufferAllocator&) = delete;

	private:
		
		void AllocateMemoryForBuffer(uint32_t InSizeBytes, void*& OutCpuPtr, D3D12_GPU_VIRTUAL_ADDRESS& OutGpuPtr);
				

		Mox::D3D12Resource& m_Resource;

		size_t m_TotalAllocationSize = 0;

		size_t m_CurrentRelativeAllocationOffset = 0;

		// Queue of relative frame offset starts. 
		// When a new frame starts, an offset is pushed back.
		// When a new frame ends, an offset is popped from front.
		// When we allocate circularly for a frame, the current offset needs to always be relatively behind the front() element of this queue.
		std::deque<size_t> m_RelativeFrameOffsetStarts;

		void* m_ResourceCpuPtr;
		D3D12_GPU_VIRTUAL_ADDRESS m_ResourceGpuPtr;

		std::vector<std::unique_ptr<Mox::D3D12BufferResource>> m_AllocatedBufferResources;
	};


}

#endif // D3D12BufferAllocator_h__

