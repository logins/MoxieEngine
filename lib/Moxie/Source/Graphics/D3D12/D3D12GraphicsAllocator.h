/*
 D3D12GraphicsAllocator.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef D3D12GraphicsAllocator_h__
#define D3D12GraphicsAllocator_h__

#include <deque>
#include <memory> // for std::unique_ptr
#include "d3d12.h"
#include "GraphicsAllocator.h"

namespace Mox { 

	class D3D12DescriptorHeap;
	class D3D12LinearBufferAllocator;
	class D3D12DescHeapFactory;
	class Window;
	struct WindowInitInput;
	class CommandQueue;

class D3D12GraphicsAllocator : public Mox::GraphicsAllocatorBase
{
public:
	// Note: The non-inline definition of the constructor is necessary to forward classes used in member smart pointers!!
	// The constructor does NEED to know the complete type, hence it cannot be inlined !!
	// The reason is the constructor also needs to know the destructors of the classes hold by the unique pointers, so that when an exception throws,
	// the class will be able to roll-back initialization of all the members.
	// More info in this thread: https://stackoverflow.com/questions/27336779/unique-ptr-and-forward-declaration
	D3D12GraphicsAllocator();

	// Note: As for the constructor, the destructor also need to know the class complete type, hence it cannot be inlined !!
	// Note2: We need to declare base destructor as virtual to make the derived class destructor to be executed first
	virtual ~D3D12GraphicsAllocator() override;

	virtual void Initialize() override;

	virtual void OnNewFrameStarted() override;

	virtual Mox::Resource& AllocateEmptyResource() override;

	virtual Mox::Buffer& AllocateBufferResource(size_t InSize, Mox::RESOURCE_HEAP_TYPE InHeapType, Mox::RESOURCE_STATE InState, Mox::RESOURCE_FLAGS InFlags = RESOURCE_FLAGS::NONE) override;

	virtual Mox::DynamicBuffer& AllocateDynamicBuffer() override;

	virtual Mox::Texture& AllocateTextureFromFile(wchar_t const* InTexturePath, Mox::TEXTURE_FILE_FORMAT InFileFormat, int32_t InMipsNum = 0, Mox::RESOURCE_FLAGS InCreationFlags = RESOURCE_FLAGS::NONE) override;

	virtual Mox::Texture& AllocateEmptyTexture(uint32_t InWidth, uint32_t InHeight, Mox::TEXTURE_TYPE InType, Mox::BUFFER_FORMAT InFormat, uint32_t InArraySize, uint32_t InMipLevels) override;

	virtual void AllocateBufferCommittedResource(Mox::CommandList& InCmdList, Mox::Resource& InDestResource, Mox::Resource& InIntermediateResource, size_t InNunElements, size_t InElementSize, const void* InBufferData, Mox::RESOURCE_FLAGS InFlags = Mox::RESOURCE_FLAGS::NONE) override;

	virtual Mox::VertexBufferView& AllocateVertexBufferView() override;

	virtual Mox::IndexBufferView& AllocateIndexBufferView() override;

	virtual Mox::ConstantBufferView& AllocateConstantBufferView(Mox::Buffer& InResource) override;

	virtual Mox::ConstantBufferView& AllocateConstantBufferView() override;

	virtual Mox::ShaderResourceView& AllocateShaderResourceView(Mox::Texture& InTexture) override;

	virtual Mox::ShaderResourceView& AllocateSrvTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, uint32_t InMostDetailedMip = 0, int32_t InMipLevels = -1, uint32_t InFirstArraySlice = 0, uint32_t InPlaceSlice = 0) override;

	virtual Mox::UnorderedAccessView& AllocateUavTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, int32_t InMipSlice = -1, uint32_t InFirstArraySlice = 0, uint32_t InPlaceSlice = 0) override;

	virtual Mox::Shader& AllocateShader(wchar_t const* InShaderPath) override;

	virtual Mox::PipelineState& AllocatePipelineState() override;

	void ReserveDynamicBufferMemory(size_t InSize, void*& OutCpuPtr, D3D12_GPU_VIRTUAL_ADDRESS& OutGpuPtr);

	D3D12DescriptorHeap& GetCpuHeap();

	D3D12DescriptorHeap& GetGpuHeap();

	virtual Mox::Window& AllocateWindow(Mox::WindowInitInput& InWindowInitInput) override;

	virtual Mox::CommandQueue& AllocateCommandQueue(class Device& InDevice, COMMAND_LIST_TYPE InCmdListType) override;

private:
	std::deque<std::unique_ptr<Mox::Resource>> m_ResourceArray;
	std::deque<std::unique_ptr<Mox::VertexBufferView>> m_VertexViewArray;
	std::deque<std::unique_ptr<Mox::IndexBufferView>> m_IndexViewArray;
	std::deque<std::unique_ptr<Mox::Shader>> m_ShaderArray;
	std::deque<std::unique_ptr<Mox::PipelineState>> m_PipelineStateArray;
	std::deque<std::unique_ptr<Mox::Window>> m_WindowArray;
	std::deque<std::unique_ptr<Mox::CommandQueue>> m_CommandQueueArray;

	std::unique_ptr<Mox::D3D12LinearBufferAllocator> m_DynamicBufferAllocator;

	std::unique_ptr<Mox::D3D12DescHeapFactory> m_DescHeapFactory;
	uint64_t m_FrameCounter = 0;
};
	

}

#endif // D3D12GraphicsAllocator_h__