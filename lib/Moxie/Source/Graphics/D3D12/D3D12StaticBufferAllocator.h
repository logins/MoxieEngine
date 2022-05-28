/*
 D3D12StaticBufferAllocator.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef D3D12StaticBufferAllocator_h__
#define D3D12StaticBufferAllocator_h__

#include "RangeAllocators.h"
#include "D3D12MoxUtils.h"

namespace Mox {

/* Allocates buffer resources as sub-allocations from a graphics resource in default heap. */
class D3D12StaticBufferAllocator
{
public:
	D3D12StaticBufferAllocator(Mox::D3D12Resource& InTargetResource, Mox::D3D12Resource& InStagingResource);

	Mox::BufferResource& Allocate(const Mox::BufferResourceRequest& InRequest);

	// Sends a command to update buffer resources with the given updates.
	// Note: the buffer updates are considered to be in memory up until the command list finishes executing the command.
	void UploadContentUpdates(Mox::CommandList& InCmdList, const std::vector<Mox::BufferResourceUpdate>& InUpdates);


private:
	// Resource in dedicated memory (default heap) to contain buffers content
	Mox::D3D12Resource& m_Resource;
	// Staging resource in upload memory (upload heap) to serve as a bridge between CPU and reserved memory
	// It will be used only when updating buffer content
	Mox::D3D12Resource& m_IntermediateResource;

	uint32_t m_ResourceAlignment;

	Mox::StaticRangeAllocator m_RangeAllocator;

	std::vector<std::unique_ptr<Mox::BufferResource>> m_AllocatedBufferResources;

	std::unordered_map<Mox::BufferResource*, uint32_t> m_BufferOffsetMap;
};

}

#endif // D3D12StaticBufferAllocator_h__
