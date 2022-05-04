/*
 GraphicsTypes.h

 First DX12 Renderer - https://github.com/logins/FirstDX12Renderer

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef GraphicsTypes_h__
#define GraphicsTypes_h__

#include <type_traits>
#include <string>
#include <vector>

namespace std {
enum class byte : unsigned char;
}


namespace Mox {

	class CommandList;

	enum class PRIMITIVE_TOPOLOGY_TYPE : int32_t 
	{
		PTT_UNDEFINED = 0,
		PTT_POINT = 1,
		PTT_LINE = 2,
		PTT_TRIANGLE = 3,
		PTT_PATCH = 4
	};

	enum class PRIMITIVE_TOPOLOGY : int32_t
	{
		PT_UNDEFINED = 0,
		PT_POINTLIST = 1,
		PT_LINELIST = 2,
		PT_LINESTRIP = 3,
		PT_TRIANGLELIST = 4,
		PT_TRIANGLESTRIP = 5
		// Eventually adding more upon need
	};

	enum class RESOURCE_FLAGS : int
	{
		NONE = 0,
		ALLOW_RENDER_TARGET = 0x1,
		ALLOW_DEPTH_STENCIL = 0x2,
		ALLOW_UNORDERED_ACCESS = 0x4,
		DENY_SHADER_RESOURCE = 0x8,
		ALLOW_CROSS_ADAPTER = 0x10,
		ALLOW_SIMULTANEOUS_ACCESS = 0x20,
		VIDEO_DECODE_REFERENCE_ONLY = 0x40
	};
	// Defining bitwise OR operator with RESOURCE_FLAGS so that the enum values can be used as flags
	inline RESOURCE_FLAGS operator | (RESOURCE_FLAGS InLeftValue, RESOURCE_FLAGS InRightValue)
	{
		using T = std::underlying_type<RESOURCE_FLAGS>::type;
		return static_cast<RESOURCE_FLAGS>(static_cast<T>(InLeftValue) | static_cast<T>(InRightValue));
	}
	inline RESOURCE_FLAGS& operator |= (RESOURCE_FLAGS& InLeftValue, RESOURCE_FLAGS InRightValue)
	{
		using T = std::underlying_type<RESOURCE_FLAGS>::type;
		InLeftValue = InLeftValue | InRightValue;
		return InLeftValue;
	}
	inline bool operator & (RESOURCE_FLAGS InLeftValue, RESOURCE_FLAGS InRightValue)
	{
		using T = std::underlying_type<RESOURCE_FLAGS>::type;
		return static_cast<T>(InLeftValue) & static_cast<T>(InRightValue);
	}

enum class COMMAND_LIST_TYPE : int {
	COMMAND_LIST_TYPE_DIRECT = 0,
	COMMAND_LIST_TYPE_BUNDLE = 1,
	COMMAND_LIST_TYPE_COMPUTE = 2,
	COMMAND_LIST_TYPE_COPY = 3,

	UNDEFINED
};

struct CpuDescHandle {
	bool IsBound = false;
};

struct GPU_DESC_HANDLE {
	bool IsBound = false;
};

struct GPUVirtualAddress {
	bool IsValid = false;
};

enum class BUFFER_FORMAT : int {
	R16_UINT, // Single channel 16 bits
	R32G32B32_FLOAT,
	R8G8B8A8_UNORM,
	D32_FLOAT,
	BC1_UNORM
};

enum class TEXTURE_FILE_FORMAT : int {
	DDS,
	PNG,
	JPG
};

enum class RESOURCE_HEAP_TYPE : int {
	DEFAULT = 0,
	UPLOAD = 1
};

enum class RESOURCE_STATE : int {
	PRESENT = 0,
	RENDER_TARGET,
	COPY_SOURCE,
	COPY_DEST,
	GEN_READ
};

enum class TEXTURE_TYPE : int {
	TEX_1D,
	TEX_2D,
	TEX_CUBE,
	TEX_3D
};

using GPU_V_ADDRESS = uint64_t; // TODO this should depend on graphics API.. as it is valid for D3D12 specifically

/*
A Resource can either be a buffer or a texture (and their specifications)
*/
struct Resource {
	uint32_t GetSize() const { return m_DataSize; }
	uint32_t GetAlignSize() const { return m_AlignmentSize; }

	void* GetData() { return m_Data.data(); }
	GPU_V_ADDRESS GetGpuData() const { return m_GpuPtr; }

	virtual void Map(void** OutCpuPp) = 0;
	virtual void UnMap() = 0;


	uint32_t GetElementsCount() { return m_ElementsNum; }


	virtual void SetCpuData(const void* InData, size_t InSize) = 0;
	
protected:
	std::vector<std::byte> m_Data;
	GPU_V_ADDRESS m_GpuPtr;
	uint32_t m_DataSize;
	uint32_t m_AlignmentSize;
	uint32_t m_ElementsNum;

};

class VertexBuffer
{
public:
	// Note: InSize = _countof(m_VertexData) * sizeof(DataType)
	VertexBuffer(Mox::CommandList& InCmdList, const void* InData, uint32_t InStride, uint32_t InSize);

	// Note: Now it takes the whole resource, but when placed resources will be implemented, this will change
	uint32_t GetSize() const { return m_BufferResource.GetSize(); }
	void* GetData() const { return m_BufferResource.GetData(); }
	GPU_V_ADDRESS GetGpuData() const { return m_BufferResource.GetGpuData(); }
	uint32_t GetStride() const { return m_Stride; }
private:
	Mox::Resource& m_BufferResource;
	uint32_t m_Stride;
};

class IndexBuffer
{
public:
	// Note: InSize = _countof(m_VertexData) * sizeof(DataType)
	IndexBuffer(Mox::CommandList& InCmdList, const void* InData, int32_t InSize, int32_t InElementsNum);
	// Note: Now it takes the whole resource, but when placed resources will be implemented, this will change
	int32_t GetSize() const { return m_BufferResource.GetSize(); }
	void* GetData() const { return m_BufferResource.GetData(); }
	GPU_V_ADDRESS GetGpuData() const { return m_BufferResource.GetGpuData(); }
	int32_t GetElementsNum() const { return m_ElementsNum; }
private:
	int32_t m_ElementsNum;
	Mox::Resource& m_BufferResource;
};


enum class BUFFER_TYPE : int8_t
{
	STATIC = 0,
	// Dynamic buffers will be allocated on circular allocator and a new version will be made every frame
	// therefore are suited for small content that updates very frequently, such as mvp matrices.
	DYNAMIC
};

struct ConstantBufferView;

// It abstracts the concept of constant buffer that we can bind to the render pipeline
struct Buffer {
	// Constructor for buffer used as a whole resource
	Buffer(Mox::BUFFER_TYPE InBufType, Mox::Resource& InResource);

	// Constructor for buffer allocating part of the resource
	Buffer(Mox::BUFFER_TYPE InBufType, Mox::Resource& InResource, void* InCpuPtr, Mox::GPU_V_ADDRESS InGpuPtr, uint32_t InSize);

	void* GetData() const { return m_CpuPtr; }

	GPU_V_ADDRESS GetGpuPtr() const { return m_GpuPtr; }

	uint32_t GetSize() const { return m_Size; }

	// Enqueues a data change for later be applied by the render thread
	void SetData(const void* InData, int32_t InSize);

	// To be executed only by the thread that owns the buffer data. In most cases, the render thread.
	void SetData_RenderThread(const void* InData, int32_t InSize) { memcpy(m_LocalData.data(), InData, InSize); }

	// Move Cpu and Gpu references to specified location and update default view

	void CopyLocalDataToLocation(void* InCpuPtr, GPU_V_ADDRESS InGpuPtr);

	Mox::Resource& GetResource() const { return m_OwningResource; }

	uint32_t GetReferencedResourceSize() const { GetResource().GetSize(); }

	// Returns the default constant buffer view associated with this buffer
	inline Mox::ConstantBufferView* GetView() { return m_DefaultView; }

protected:
	Mox::BUFFER_TYPE m_BufferType;
	// Note: the following is a Sub-Allocation in the resource (most of the times will cover just a portion of a resource)
	Mox::Resource& m_OwningResource;
	void* m_CpuPtr;
	GPU_V_ADDRESS m_GpuPtr;
	uint32_t m_Size;
	std::vector<std::byte> m_LocalData;

	Mox::ConstantBufferView* m_DefaultView;
};

// Stores an update for a constant buffer to be then applied by the render thread
struct ConstantBufferUpdate
{
	ConstantBufferUpdate(Mox::Buffer& InBuffer, const void* InData, uint32_t InSize)
		: m_Buffer(InBuffer), UpdateData(std::vector<std::byte>(InSize))
	{
		memcpy(UpdateData.data(), InData, InSize);
	}
	Mox::Buffer& m_Buffer;
	std::vector<std::byte> UpdateData;

	// Note: This will have to be executed by the render thread
	inline void ApplyUpdate()
	{
		m_Buffer.SetData_RenderThread(UpdateData.data(), UpdateData.size());
	}
};

struct Texture {

	Texture(Mox::Resource& InRes) : m_ReferencedResource(InRes) { }

	virtual void UploadToGPU(Mox::CommandList& InCommandList, Mox::Buffer& InIntermediateBuffer) = 0;
	// Allocate empty space on GPU
	virtual void InstantiateOnGPU() = 0;

	virtual size_t GetGPUSize() = 0;

	size_t GetWidth() const { return m_Width; }
	size_t GetHeight() const { return m_Height; }
	BUFFER_FORMAT GetFormat() { return m_TexelFormat; }
	TEXTURE_TYPE GetType() { return m_Type; }
	size_t GetMipLevelsNum() { return m_MipLevelsNum; }
	size_t GetArraySize() const { return m_ArraySize; }

	Mox::Resource& GetResource() const { return m_ReferencedResource; }

protected:
	Mox::Resource&	m_ReferencedResource;
	size_t          m_Width;
	size_t          m_Height;     // Should be 1 for 1D textures
	size_t          m_ArraySize;  // For cubemap, this is a multiple of 6
	size_t          m_MipLevelsNum;
	BUFFER_FORMAT   m_TexelFormat;
	TEXTURE_TYPE    m_Type;
};

struct ResourceView
{
	// Views that are not Gpu allocated at the time of getting considered for drawing inside a pass
	// will be considered dynamic and a gpu descriptor will be allocated for them on the spot
	virtual bool IsGpuAllocated() = 0;
};

struct ConstantBufferView : public ResourceView {

	virtual void ReferenceBuffer(Mox::Buffer& InBuffer) = 0;

	virtual void RebuildResourceReference() = 0;
protected:
	ConstantBufferView(Mox::Buffer& InBufffer) : m_ReferencedBuffer(InBufffer) { };

	Mox::Buffer m_ReferencedBuffer;
};

struct ShaderResourceView : public ResourceView {
	virtual void InitAsTex2DOrCubemap(Mox::Texture& InTexture) = 0;
protected:
	ShaderResourceView(Mox::Resource& InResource) { }
};

struct UnorderedAccessView : public ResourceView {
	UnorderedAccessView(Mox::Resource& InReferencedResource) { }
};

struct VertexBufferView {
public:
	virtual void ReferenceResource(Mox::VertexBuffer& InVB) = 0;
protected:
	VertexBufferView(Mox::VertexBuffer& InVB) : m_ReferencedVB(InVB) {}
	Mox::VertexBuffer& m_ReferencedVB;
};

struct IndexBufferView {
public:
	virtual void ReferenceResource(Mox::IndexBuffer& InIB, Mox::BUFFER_FORMAT InFormat) = 0;
	inline Mox::IndexBuffer& GetIB() const { return m_ReferencedIB; }
protected:
	IndexBufferView(Mox::IndexBuffer& InIB) : m_ReferencedIB(InIB) {}
	Mox::IndexBuffer& m_ReferencedIB;
};

enum class SAMPLE_FILTER_TYPE : int {
	POINT,
	LINEAR,
	ANISOTROPIC
};

enum class TEXTURE_ADDRESS_MODE : int {
	WRAP,
	MIRROR,
	CLAMP,
	BORDER
};

struct StaticSampler {
	StaticSampler(uint32_t InShaderRegister, SAMPLE_FILTER_TYPE InFilterType)
		: m_ShaderRegister(InShaderRegister), m_Filter(InFilterType), 
		m_AddressU(TEXTURE_ADDRESS_MODE::WRAP),	m_AddressV(TEXTURE_ADDRESS_MODE::WRAP),	m_AddressW(TEXTURE_ADDRESS_MODE::WRAP)
	{ }
	StaticSampler(uint32_t InShaderRegister, SAMPLE_FILTER_TYPE InFilterType, TEXTURE_ADDRESS_MODE InAddressMode)
		: m_ShaderRegister(InShaderRegister), m_Filter(InFilterType),
		m_AddressU(InAddressMode), m_AddressV(InAddressMode), m_AddressW(InAddressMode)
	{ }
	uint32_t m_ShaderRegister;
	SAMPLE_FILTER_TYPE m_Filter;
	TEXTURE_ADDRESS_MODE m_AddressU;
	TEXTURE_ADDRESS_MODE m_AddressV;
	TEXTURE_ADDRESS_MODE m_AddressW;
};

struct Rect {
	Rect(int32_t InLeft, int32_t InTop, int32_t InRight, int32_t InBottom) {}
protected:
	Rect(){}
};

struct ViewPort {
	ViewPort(float InTopLeftX, float InTopLeftY, float InWidth, float InHeight) {}
	virtual void SetWidthAndHeight(float InWidth, float InHeight) {}
protected:
	ViewPort(){}
};


enum class SHADER_VISIBILITY {
	SV_ALL, SV_VERTEX, SV_HULL, SV_DOMAIN, SV_GEOMETRY, SV_PIXEL
};

struct Shader {
protected:
	Shader() = default;
};


}

#endif // GraphicsTypes_h__
