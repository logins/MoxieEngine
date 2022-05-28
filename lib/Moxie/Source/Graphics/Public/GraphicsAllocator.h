/*
 GraphicsAllocator.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef GraphicsAllocator_h__
#define GraphicsAllocator_h__

#include "GraphicsTypes.h"
#include "PipelineState.h"

namespace Mox { 

	class Device;
	class CommandList;
	class Window;
	struct WindowInitInput;
	class CommandQueue;
	class RenderProxy;

// This pure virtual class serves as interface for any Graphics Allocator we want to implement.
// A graphics allocator is meant to handle the lifetime of graphics resources (buffers, textures and descriptors) 
// and having a direct communication with GPU memory.
// In Application code we are going to use GraphicsAllocator with a factory pattern class, so we can call GraphicsAllocator::Get()
// to retrieve the real allocator (in our case a D3D12GraphicsAllocator object).
// Using this pattern is better compared to a singleton, because we are then able to extend the graphics allocator class
// to different platform-agnostic implementations and possibly different versions for a specific platform (e.g. a specific implementation for testing).
class GraphicsAllocatorBase
{
public:
	GraphicsAllocatorBase();
	
	virtual ~GraphicsAllocatorBase();

	// Creates default resources
	virtual void Initialize() = 0;

	virtual void OnNewFrameStarted() = 0;

	virtual void OnNewFrameEnded() = 0;

	virtual void UpdateStaticResources(Mox::CommandList& InCmdList, const std::vector<Mox::BufferResourceUpdate>& InUpdates) = 0;

	virtual Mox::VertexBuffer& AllocateVertexBuffer(const void* InData, uint32_t InStride, uint32_t InSize) = 0;

	virtual Mox::IndexBuffer& AllocateIndexBuffer(const void* InData, uint32_t InStride, uint32_t InSize) = 0;

	virtual Mox::BufferResource& AllocateDynamicBuffer(uint32_t InSize) = 0;

	virtual Mox::Texture& AllocateTextureFromFile(wchar_t const* InTexturePath, Mox::TEXTURE_FILE_FORMAT InFileFormat, int32_t InMipsNum = 0, Mox::RESOURCE_FLAGS InCreationFlags = RESOURCE_FLAGS::NONE) = 0;
	
	virtual Mox::Texture& AllocateEmptyTexture(uint32_t InWidth, uint32_t InHeight, Mox::TEXTURE_TYPE InType, Mox::BUFFER_FORMAT InFormat, uint32_t InArraySize, uint32_t InMipLevels) = 0;

	virtual std::vector<RenderProxy*> CreateProxies(const std::vector<Mox::RenderProxyRequest>& InRequests) = 0;

	virtual void AllocateResourceForBuffer(const Mox::BufferResourceRequest& InResourceRequest) = 0;

	// Preferable for Static Buffers such as Vertex and Index Buffers.
	// First creates an intermediary buffer in shared memory (upload heap), then the same buffer in reserved memory (default heap)
	// and then calls UpdateSubresources that will copy the content of the first buffer in the second one.
	// The type of allocation is Committed Resource, meaning that a resource heap will be created specifically to contain the allocated resource each time.

	virtual Mox::VertexBufferView& AllocateVertexBufferView(Mox::BufferResource& InVBResource) = 0;
	virtual Mox::IndexBufferView& AllocateIndexBufferView(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum) = 0;
	virtual Mox::ConstantBufferView& AllocateConstantBufferView(Mox::BufferResource& InResource) = 0;

	virtual Mox::ShaderResourceView& AllocateShaderResourceView(Mox::Texture& InTexture) = 0;
	// SRV referencing a Tex2D Array
	virtual Mox::ShaderResourceView& AllocateSrvTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, uint32_t InMostDetailedMip = 0, int32_t InMipLevels = -1, uint32_t InFirstArraySlice = 0, uint32_t InPlaceSlice = 0) = 0;

	// UAV referencing a Tex2D Array
	virtual Mox::UnorderedAccessView& AllocateUavTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, int32_t InMipSlice = -1, uint32_t InFirstArraySlice = 0, uint32_t InPlaceSlice = 0) = 0;
	
	virtual Mox::Shader& AllocateShader(wchar_t const* InShaderPath) = 0;
	virtual Mox::PipelineState& AllocatePipelineState() = 0;

	virtual Mox::Window& AllocateWindow(Mox::WindowInitInput& InWindowInitInput) = 0;

	virtual Mox::CommandQueue& AllocateCommandQueue(class Device& InDevice, COMMAND_LIST_TYPE InCmdListType) = 0;






	// Deleting copy constructor, assignment operator, move constructor and move assignment
	GraphicsAllocatorBase(const GraphicsAllocatorBase&) = delete;
	GraphicsAllocatorBase& operator=(const GraphicsAllocatorBase&) = delete;
	GraphicsAllocatorBase(GraphicsAllocatorBase&&) = delete;
	GraphicsAllocatorBase& operator=(GraphicsAllocatorBase&&) = delete;
};



class GraphicsAllocator
{
public:
	// Will return the instance of the chosen graphics allocator for the current configuration (for now only the DX12 one)
	static GraphicsAllocatorBase* Get() { return m_DefaultInstance; };

	static std::unique_ptr<GraphicsAllocatorBase> CreateInstance();

	// Sets the default reference to be called with the Get() method.
	// This allows other systems (such as Application) to control the lifetime and take ownership of the default graphics allocator
	static void SetDefaultInstance(GraphicsAllocatorBase* InGraphicsAllocator);

	// Deleting copy constructor, assignment operator, move constructor and move assignment
	GraphicsAllocator(const GraphicsAllocator&) = delete;
	GraphicsAllocator& operator=(const GraphicsAllocator&) = delete;
	GraphicsAllocator(GraphicsAllocator&&) = delete;
	GraphicsAllocator& operator=(GraphicsAllocator&&) = delete;

private:
	// This class is not intended to be instantiated
	GraphicsAllocator() = default;
	static GraphicsAllocatorBase* m_DefaultInstance;
};



}
#endif // GraphicsAllocator_h__
