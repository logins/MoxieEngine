/*
 D3D12BufferAllocator.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "D3D12BufferAllocator.h"
#include "MoxMath.h"
#include "D3D12MoxUtils.h"
#include "D3D12Device.h"
#include "MoxUtils.h"
#include "RangeAllocators.h"

namespace Mox{ 

	D3D12LinearBufferAllocator::D3D12LinearBufferAllocator(Mox::D3D12Resource& InResource)
		: m_Resource(InResource), m_TotalAllocationSize(InResource.GetSize()) // At the moment we are using the entire resource for the linear buffer allocator
	{
		m_ResourceGpuPtr = m_Resource.GetGpuData(); 

		m_ResourceCpuPtr = m_Resource.GetData();

		// Opening mapping channel with CPU
		m_Resource.Map(&m_ResourceCpuPtr);


		// Initialise indexes
		m_CurrentRelativeAllocationOffset = 0;
		m_RelativeFrameOffsetStarts.push_back(m_TotalAllocationSize-1); // This will allow correct sub-allocation boundary checks when the application starts

		// Note: Reserve is very important here, since we are returning references of the created buffers.
		// When an std::vector will resize, it will invalidate all the references!! 
		// This means if we were not to reserve data, when we reach the default reserved maximum index, 
		// the vector would automatically resize and invalidate all our previous references!
		m_AllocatedBuffers.reserve(500);
	}

	// Necessary to forward declare class type in unique_ptr member variable,
	// it prevents destructors to be inlined in which case breaks compilation
	D3D12LinearBufferAllocator::~D3D12LinearBufferAllocator()
	{
		// Closes the mappping channel with the CPU side
		m_Resource.UnMap();
	}


	Mox::BufferResource& D3D12LinearBufferAllocator::Allocate(uint32_t InSize)
	{
		uint32_t alignedSize = Mox::Align(InSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		void* cpuPtr; D3D12_GPU_VIRTUAL_ADDRESS gpuPtr;
		// Note: the InSize can be modified by the following function to check if it follows the D3D12 alignment size constraints
		AllocateMemoryForBuffer(alignedSize, cpuPtr, gpuPtr);

		m_AllocatedBuffers.emplace_back( std::make_unique<Mox::D3D12BufferResource>(m_Resource, cpuPtr, gpuPtr, alignedSize) );

		return *m_AllocatedBuffers.back().get();
	}

	void D3D12LinearBufferAllocator::OnFrameStarted()
	{
		// When frame starts, update the relative offset and copy in all the current active dynamic buffers 

		m_RelativeFrameOffsetStarts.push_back(m_CurrentRelativeAllocationOffset);

		for (auto& currrentBuffer : m_AllocatedBuffers)
		{
			void* cpuPtr;
			GPU_V_ADDRESS gpuPtr;
			// We first reserve the new memory block for the buffer
			AllocateMemoryForBuffer(currrentBuffer->GetSize(), cpuPtr, gpuPtr);
			// Then we copy the previous frame data to the new location
			currrentBuffer->CopyLocalDataToLocation(cpuPtr, gpuPtr);

		}


	}

	void D3D12LinearBufferAllocator::AllocateMemoryForBuffer(uint32_t InSizeBytes, void*& OutCpuPtr, D3D12_GPU_VIRTUAL_ADDRESS& OutGpuPtr)
	{
		size_t candidateOffsetStart = Mox::Align(m_CurrentRelativeAllocationOffset+1, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);//m_CurrentRelativeAllocationOffset; 

		size_t candidateOffsetEnd = candidateOffsetStart + InSizeBytes;

		// First checking if we are going beyond the maximum allocation size
		if (candidateOffsetEnd > m_TotalAllocationSize)
		{
			// If so, we start from the beginning
			candidateOffsetStart = 0;
			candidateOffsetEnd = InSizeBytes;
		}


		// Allocation offset end needs to be outside the taken memory interval
		Check(candidateOffsetEnd < m_RelativeFrameOffsetStarts.front() || candidateOffsetStart > m_CurrentRelativeAllocationOffset)

		m_CurrentRelativeAllocationOffset = candidateOffsetEnd;

		OutCpuPtr = static_cast<uint8_t*>(m_ResourceCpuPtr) + candidateOffsetStart; // CPU pointer for the start of the allocation

		OutGpuPtr = m_ResourceGpuPtr + candidateOffsetStart; // GPU pointer for the start of the allocation

	}


	void D3D12LinearBufferAllocator::Reset()
	{
		m_TotalAllocationSize = 0;

		m_CurrentRelativeAllocationOffset = 0;

		m_RelativeFrameOffsetStarts.clear();
	}

}