/*
 D3D12TextureAllocator.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef D3D12TextureAllocator_h__
#define D3D12TextureAllocator_h__

namespace Mox {

struct D3D12Resource;
class TextureResource;
class StaticRangeAllocator;

class D3D12TextureAllocator
{
public:
	D3D12TextureAllocator(size_t InHeapSize, Mox::D3D12Resource& InStagingResource);

	Mox::TextureResource& Allocate(const TextureResourceDesc& InDesc);

	void UpdateContent(Mox::CommandList& InCmdList, const std::vector<Mox::TextureResourceUpdate>& InTexUpdates);

private:
	// Default heap where placed texture resources will be allocated on
	Microsoft::WRL::ComPtr<ID3D12Heap> m_OwningHeap;

	// Located in upload memory (upload heap) to serve as a bridge between CPU and reserved memory
	// It will be used only when updating buffer content
	Mox::D3D12Resource& m_StagingResource;
	// Maximum size in bytes
	size_t m_PoolSize;
	size_t m_AllocationsAlignment;

	std::vector<std::unique_ptr<Mox::TextureResource>> m_TextureArray;

	std::unique_ptr<Mox::StaticRangeAllocator> m_RangeAllocator;
};

}

#endif // D3D12TextureAllocator_h__
