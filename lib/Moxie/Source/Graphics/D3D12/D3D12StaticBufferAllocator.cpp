/*
 D3D12StaticBufferAllocator.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "D3D12StaticBufferAllocator.h"
#include "MoxMath.h"
#include "D3D12CommandList.h"

namespace Mox {

D3D12StaticBufferAllocator::D3D12StaticBufferAllocator(Mox::D3D12Resource& InTargetResource, Mox::D3D12Resource& InStagingResource) : m_Resource(InTargetResource), m_IntermediateResource(InStagingResource),
	m_RangeAllocator(0, InTargetResource.GetSize()), m_ResourceAlignment(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
{

}

Mox::BufferResource& D3D12StaticBufferAllocator::Allocate(const Mox::BufferResourceRequest& InRequest)
{
	// Note: The passed size will be aligned before asking for allocation.
	// This will ensure that the returned allocation offset will be aligned.
	uint32_t relativeResourceOffset = m_RangeAllocator.AllocateRange(Mox::Align(InRequest.m_AllocationSize, m_ResourceAlignment));

	m_AllocatedBufferResources.emplace_back(
		std::make_unique<Mox::D3D12BufferResource>(InRequest.m_ContentType, BUFFER_ALLOC_TYPE::STATIC, m_Resource,
			nullptr/*static_cast<uint8_t*>(m_Resource.GetData()) + relativeResourceOffset*/, m_Resource.GetGpuData() + relativeResourceOffset,
			InRequest.m_AllocationSize, InRequest.m_Stride));

	Mox::BufferResource* outResPtr = m_AllocatedBufferResources.back().get();

	m_BufferOffsetMap[outResPtr] = relativeResourceOffset;

	return *outResPtr;
}

void D3D12StaticBufferAllocator::UploadContentUpdates(Mox::CommandList& InCmdList, const std::vector<Mox::BufferResourceUpdate>& InUpdates)
{
	// Map intermediate resource, copy content in it, unmap it, 
	// then copy buffer region to transfer changes from the intermediate resource to the target resource
	std::unordered_map<Mox::BufferResource*, uint32_t> intermediateAllocationOffsetMap;
	uint32_t currentAllocOffset = 0;
	uint8_t* mappedPtr = nullptr;
	m_IntermediateResource.Map(&static_cast<void*>(mappedPtr));
	for (const Mox::BufferResourceUpdate& bufUpdate : InUpdates)
	{
		intermediateAllocationOffsetMap[bufUpdate.m_BufResHolder->GetResource()] = currentAllocOffset;
		memcpy(static_cast<void*>(mappedPtr + currentAllocOffset),
			bufUpdate.m_UpdateData.data(), bufUpdate.m_UpdateData.size());
		currentAllocOffset += Mox::Align(bufUpdate.m_UpdateData.size(), m_ResourceAlignment);
	}

	m_IntermediateResource.UnMap();

	
	Mox::TransitionInfoVector resBarriers{
		{&m_Resource,Mox::RESOURCE_STATE::GEN_READ, Mox::RESOURCE_STATE::COPY_DEST}
	};

	static_cast<Mox::D3D12CommandList&>(InCmdList).ResourceBarriers(resBarriers);

	// Here we expect every buffer to already have its own graphics resource
	for (const Mox::BufferResourceUpdate& bufUpdate : InUpdates)
	{
		Mox::BufferResource* curBufResource = bufUpdate.m_BufResHolder->GetResource();
		Check(m_BufferOffsetMap.find(curBufResource) != m_BufferOffsetMap.end())
		static_cast<Mox::D3D12CommandList&>(InCmdList).GetInner()->CopyBufferRegion(
				m_Resource.GetInner().Get(), m_BufferOffsetMap[curBufResource],
				m_IntermediateResource.GetInner().Get(), intermediateAllocationOffsetMap[curBufResource],
				bufUpdate.m_UpdateData.size());
	}

	resBarriers = Mox::TransitionInfoVector {
		{&m_Resource,Mox::RESOURCE_STATE::COPY_DEST, Mox::RESOURCE_STATE::GEN_READ}
	};

	static_cast<Mox::D3D12CommandList&>(InCmdList).ResourceBarriers(resBarriers);

}

}

