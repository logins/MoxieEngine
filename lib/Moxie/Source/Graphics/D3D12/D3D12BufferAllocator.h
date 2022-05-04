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
	struct Buffer;
	struct D3D12Buffer;

	/*
	D3D12LinearBufferAllocator performs constant buffer sub-allocations in a single buffer resource.
	This will operate in a ring buffer manner similar to what is described here https://www.codeproject.com/Articles/1094799/Implementing-Dynamic-Resources-with-Direct-D
	*/
	class D3D12LinearBufferAllocator {

	public:
		D3D12LinearBufferAllocator(Mox::D3D12Resource& InResource);

		~D3D12LinearBufferAllocator();

		// Note: We do not need to pass the alignment since it is decided by the hosting resource
		Mox::Buffer& Allocate(uint32_t InSize);



		// When frame starts, update the relative offset and copy in all the current active dynamic buffers 
		void OnFrameStarted();

		// When a frame ends, remove the oldest recorded offset
		void OnFrameEnded() { m_RelativeFrameOffsetStarts.pop_front(); }

		void Reset();

		// Do not allow copy construct
		D3D12LinearBufferAllocator(const D3D12LinearBufferAllocator& ) = delete;
		// Do not allow copy assignment
		D3D12LinearBufferAllocator& operator=(const D3D12LinearBufferAllocator&) = delete;

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

		std::vector<Mox::D3D12Buffer> m_AllocatedBuffers;
	};


}

#endif // D3D12BufferAllocator_h__

