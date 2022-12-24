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
#include "D3D12MoxUtils.h"

namespace Mox { 

	class D3D12DescriptorHeap;
	class D3D12DynamicBufferAllocator;
	class D3D12StaticBufferAllocator;
	class D3D12TextureAllocator;
	class D3D12DescHeapFactory;
	class Window;
	struct WindowInitInput;
	class CommandQueue;
	class Drawable;

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

	void Initialize(Mox::CommandList& InCmdList) override;

	void OnNewFrameStarted() override;

	void OnNewFrameEnded() override;

	void UpdateStaticBufferResources(Mox::CommandList& InCmdList, const std::vector<Mox::BufferResourceUpdate>& InUpdates) override;

	void UpdateTextureResources(Mox::CommandList& InCmdList, const std::vector<Mox::TextureResourceUpdate>& InTextureUpdates) override;

	Mox::VertexBuffer& AllocateVertexBuffer(const Mox::INPUT_LAYOUT_DESC& InLayoutDesc, const void* InData, uint32_t InStride, uint32_t InSize) override;

	Mox::IndexBuffer& AllocateIndexBuffer(const void* InData, uint32_t InStride, uint32_t InSize) override;

	Mox::BufferResource& AllocateDynamicBuffer(uint32_t InSize) override;

	void AllocateResourceForTexture(const Mox::TextureResourceRequest& InTexResRequest) override;

	Mox::VertexBufferView& AllocateVertexBufferView(Mox::BufferResource& InVBResource) override;

	Mox::IndexBufferView& AllocateIndexBufferView(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum) override;

	Mox::ConstantBufferView& AllocateConstantBufferView(Mox::BufferResource& InResource) override;

	Mox::ShaderResourceView& AllocateShaderResourceView(Mox::TextureResource& InTexture) override;

	Mox::ShaderResourceView& AllocateSrvTex2DArray(Mox::TextureResource& InTexture, uint32_t InArraySize, uint32_t InMostDetailedMip = 0, int32_t InMipLevels = -1, uint32_t InFirstArraySlice = 0, uint32_t InPlaceSlice = 0) override;

	Mox::UnorderedAccessView& AllocateUavTex2DArray(Mox::TextureResource& InTexture, uint32_t InArraySize, int32_t InMipSlice = -1, uint32_t InFirstArraySlice = 0, uint32_t InPlaceSlice = 0) override;

	Mox::Shader& AllocateShader(wchar_t const* InShaderPath) override;

	Mox::PipelineState& AllocatePipelineState() override;

	void ReserveDynamicBufferMemory(size_t InSize, void*& OutCpuPtr, D3D12_GPU_VIRTUAL_ADDRESS& OutGpuPtr);

	D3D12DescriptorHeap& GetDescriptorsCpuHeap();

	D3D12DescriptorHeap& GetDescriptorsGpuHeap();

	virtual Mox::Window& AllocateWindow(Mox::WindowInitInput& InWindowInitInput) override;

	virtual Mox::CommandQueue& AllocateCommandQueue(class Device& InDevice, COMMAND_LIST_TYPE InCmdListType) override;


	std::vector<Mox::RenderProxy*> CreateProxies(const std::vector<Mox::RenderProxyRequest>& InRequests) override;

	void CreateDrawables(const std::vector<Mox::DrawableCreationInfo>& InRequests) override;


	void AllocateResourceForBuffer(const Mox::BufferResourceRequest& InResourceRequest) override;

	Mox::D3D12Resource& AllocateD3D12Resource(D3D12_RES_TYPE InResType, Mox::RESOURCE_HEAP_TYPE InHeapType, 
		uint32_t InSize = 1, Mox::RESOURCE_FLAGS InFlags = RESOURCE_FLAGS::NONE);

	// D3D12 Specific
	Mox::D3D12Resource& AllocateD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> InD3D12Res, D3D12_RES_TYPE InResType, size_t InSize = 1);



private:

	// TODO move this as a free function in the implementation file, to be of help on every frame when updating the dynamic buffer location and content
	void StoreAndReferenceDynamicBuffer(uint32_t InRootIdx, Mox::BufferResource& InDynBuffer, Mox::ConstantBufferView& InResourceView);

	std::deque<Mox::D3D12Resource> m_GraphicsResources;

	std::deque<Mox::VertexBuffer> m_VertexBufferArray;
	std::deque<Mox::IndexBuffer> m_IndexBufferArray;
	std::deque<Mox::BufferResource> m_BufferArray;

	std::deque<std::unique_ptr<Mox::Drawable>> m_DrawableArray;
	std::deque<std::unique_ptr<Mox::RenderProxy>> m_RenderProxyArray;

	std::deque<Mox::D3D12VertexBufferView> m_VertexViewArray;
	std::deque<Mox::D3D12IndexBufferView> m_IndexViewArray;
	std::deque<std::unique_ptr<Mox::Shader>> m_ShaderArray;
	std::deque<std::unique_ptr<Mox::PipelineState>> m_PipelineStateArray;
	std::deque<std::unique_ptr<Mox::Window>> m_WindowArray;
	std::deque<std::unique_ptr<Mox::CommandQueue>> m_CommandQueueArray;

	std::unique_ptr<Mox::D3D12StaticBufferAllocator> m_StaticBufferAllocator;

	std::unique_ptr<Mox::D3D12DynamicBufferAllocator> m_DynamicBufferAllocator;

	std::unique_ptr<Mox::D3D12TextureAllocator> m_TextureAllocator;

	std::unique_ptr<Mox::D3D12DescHeapFactory> m_DescHeapFactory;

	// Changes to be picked up by the render thread


	uint64_t m_FrameCounter = 0;
};
	

}

#endif // D3D12GraphicsAllocator_h__