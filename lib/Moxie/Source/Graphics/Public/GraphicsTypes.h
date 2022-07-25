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

	enum class PRIMITIVE_TOPOLOGY_TYPE : uint8_t
	{
		PTT_UNDEFINED = 0,
		PTT_POINT = 1,
		PTT_LINE = 2,
		PTT_TRIANGLE = 3,
		PTT_PATCH = 4
	};

	enum class PRIMITIVE_TOPOLOGY : uint8_t
	{
		PT_UNDEFINED = 0,
		PT_POINTLIST = 1,
		PT_LINELIST = 2,
		PT_LINESTRIP = 3,
		PT_TRIANGLELIST = 4,
		PT_TRIANGLESTRIP = 5
		// Eventually adding more upon need
	};

	enum class RESOURCE_FLAGS : uint8_t
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

enum class BUFFER_FORMAT : uint8_t {
	R16_UINT, // Single channel 16 bits
	R32G32B32_FLOAT,
	R8G8B8A8_UNORM,
	B8G8R8A8_UNORM,
	D32_FLOAT,
	BC1_UNORM
};

enum class TEXTURE_FILE_FORMAT : uint8_t {
	DDS,
	PNG,
	JPG
};

enum class RESOURCE_HEAP_TYPE : uint8_t {
	DEFAULT = 0,
	UPLOAD = 1
};

enum class RESOURCE_STATE : uint8_t {
	PRESENT = 0,
	RENDER_TARGET,
	COPY_SOURCE,
	COPY_DEST,
	GEN_READ
};

enum class TEXTURE_TYPE : uint8_t {
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
	uint32_t GetAlignSize() const { return m_Alignment; }

	GPU_V_ADDRESS GetGpuData() const { return m_GpuPtr; }

	virtual void Map(void** OutCpuPp) = 0;
	virtual void UnMap() = 0;

protected:
	GPU_V_ADDRESS m_GpuPtr;
	uint32_t m_DataSize;
	uint32_t m_Alignment;
};

enum class RES_CONTENT_TYPE : int8_t
{
	VERTEX,
	INDEX,
	CONSTANT,
	TEXTURE
};

enum class BUFFER_ALLOC_TYPE : int8_t
{
	STATIC = 0,
	// Dynamic buffers will be allocated on circular allocator and a new version will be made every frame
	// therefore are suited for small content that updates very frequently, such as mvp matrices.
	DYNAMIC
};

struct ResourceView;

// Graphics buffer resource meant to be owned by the render thread.
struct BufferResource {
	// Constructor for buffer used as a whole resource

	// Constructor for buffer allocating part of the resource
	BufferResource(Mox::RES_CONTENT_TYPE InContentType, Mox::BUFFER_ALLOC_TYPE InBufType, 
		Mox::Resource& InResource, void* InCpuPtr, Mox::GPU_V_ADDRESS InGpuPtr, uint32_t InSize, uint32_t InStride = 1);

	void* GetData() const { return m_CpuPtr; }

	GPU_V_ADDRESS GetGpuPtr() const { return m_GpuPtr; }

	uint32_t GetSize() const { return m_Size; }

	inline uint32_t GetStride() const { return m_Stride; }

	// Enqueues a data change for later be applied by the render thread

	// To be executed only by the thread that owns the buffer data. In most cases, the render thread.
	void SetData(const void* InData, int32_t InSize);

	// Move Cpu and Gpu references to specified location and update default view

	void CopyLocalDataToLocation(void* InCpuPtr, GPU_V_ADDRESS InGpuPtr);

	Mox::Resource& GetResource() const { return m_OwningResource; }

	uint32_t GetReferencedResourceSize() const { GetResource().GetSize(); }

	Mox::BUFFER_ALLOC_TYPE GetType() const { return m_BufferType; }

	// Returns the default constant buffer view associated with this buffer
	inline Mox::ResourceView* GetView() { return m_DefaultView; }

private:
	void CreateDefaultView();

protected:
	Mox::RES_CONTENT_TYPE m_ContentType;
	Mox::BUFFER_ALLOC_TYPE m_BufferType;
	// Note: the following is a Sub-Allocation in the resource (most of the times will cover just a portion of a resource)
	Mox::Resource& m_OwningResource;
	void* m_CpuPtr;
	GPU_V_ADDRESS m_GpuPtr;
	uint32_t m_Size;
	uint32_t m_Stride;
	int32_t m_ElementsNum;
	std::vector<std::byte> m_LocalData;

	Mox::ResourceView* m_DefaultView;

private:

	BufferResource() = delete;
	// A buffer resource is unique in its data and cannot be copied
	BufferResource& operator=(const BufferResource&) = delete;
	BufferResource(const BufferResource&) = delete;
};


struct VertexBufferView;
struct IndexBufferView;


class BufferResourceHolder
{
public:
	BufferResourceHolder(Mox::RES_CONTENT_TYPE InContentType, Mox::BUFFER_ALLOC_TYPE InAllocType, uint32_t InSize, uint32_t InStride = 0)
		: m_ContentType(InContentType), m_BufferType(InAllocType), m_Size(InSize), m_Stride(InStride) { }
	~BufferResourceHolder();
	// Setting the buffer resource needs to happen in the render thread
	// This will also allow for the creation of 
	virtual void SetBufferResource(Mox::BufferResource& InResource) = 0;

	void SetData(const void* InData, uint32_t InSize);
	inline GPU_V_ADDRESS GetGpuPtr() const { return m_Resource->GetGpuPtr(); }
	Mox::BufferResource* GetResource() const { return m_Resource; }
	inline Mox::BUFFER_ALLOC_TYPE GetAllocType() const { return m_BufferType; }
	inline Mox::RES_CONTENT_TYPE GetContentType() const { return m_ContentType; }
	inline uint32_t GetSize() const { return m_Size; }
	inline uint32_t GetStride() const { return m_Stride; }
protected:
	Mox::BUFFER_ALLOC_TYPE m_BufferType;
	Mox::RES_CONTENT_TYPE m_ContentType;
	uint32_t m_Size;
	uint32_t m_Stride;
	// Owned and accessed only by the render thread
	BufferResource* m_Resource;
};

class VertexBuffer : public BufferResourceHolder
{
public:
	// Note: InSize = _countof(m_VertexData) * sizeof(DataType)
	VertexBuffer(const void* InData, uint32_t InStride, uint32_t InSize);

	void SetBufferResource(Mox::BufferResource& InResource) override { m_Resource = &InResource; };

};

class IndexBuffer : public BufferResourceHolder
{
public:

	IndexBuffer(const void* InData, uint32_t InStride, uint32_t InSize);

	int32_t GetElementsNum() const { return m_ElementsNum; }

	void SetBufferResource(Mox::BufferResource& InResource) override { m_Resource = &InResource; };

private:

	int32_t m_ElementsNum;
};



struct ConstantBufferView;

// Representation of a graphics buffer resource owned by the main thread.
// This internally holds a BufferResource which is the effective graphics 
// resource handled by the render thread.
class ConstantBuffer : public BufferResourceHolder
{
public:
	ConstantBuffer(Mox::BUFFER_ALLOC_TYPE InType, uint32_t InSize);
	virtual ~ConstantBuffer();

	// Note: resource is meant to be accessed only by the render thread
	inline void SetBufferResource(BufferResource& InResource) override { m_Resource = &InResource; }


	// A buffer needs to be unique in its data and cannot be copied
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;
	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer() = delete;
};



struct TextureDesc
{
	size_t m_Width;
	size_t m_Height;
	size_t m_ArraySize;
	size_t m_MipLevelsNum;
	size_t m_PlanesNum;
	BUFFER_FORMAT m_TexelFormat;
	TEXTURE_TYPE m_Type;
};

struct TexDataInfo;
class TextureResource;

class Texture
{
public:
	Texture(const wchar_t* InFilePath);

	Texture(Mox::TextureDesc& InDesc);

	// Note: Updates are meant to be stored in contiguous memory.
	// After the update is done, the input memory will be deleted.
	void UpdateContent(const void* InUpdateData, size_t InUpdateSize, std::vector<Mox::TexDataInfo>& InUpdateInfo);

	// Reserved for render thread
	Mox::TextureResource* GetResource() const { return m_Resource; }
	void SetResource(Mox::TextureResource& InRes) { m_Resource = &InRes; }

private:
	// Pointer reserved for render thread access
	Mox::TextureResource* m_Resource;

	Mox::TextureDesc m_Desc;
};


struct ResourceView
{
	// Views that are not Gpu allocated at the time of getting considered for drawing inside a pass
	// will be considered dynamic and a gpu descriptor will be allocated for them on the spot
	virtual bool IsGpuAllocated() = 0;

	virtual void RebuildResourceReference() = 0;

	// Prevent accidental copy, each view should be unique
	ResourceView operator=(const ResourceView&) = delete;
	ResourceView(const ResourceView&) = delete;
	ResourceView() = default;
};

struct ConstantBufferView : public ResourceView {

	virtual void ReferenceBuffer(Mox::BufferResource& InBuffer) = 0;

	static ConstantBufferView* GetNull();
	
protected:
	ConstantBufferView();

	ConstantBufferView(Mox::BufferResource& InBufffer) : m_ReferencedBuffer(&InBufffer) { };

	Mox::BufferResource* m_ReferencedBuffer;

	// Derived platform-specific class will need to initialize it
	static std::unique_ptr<ConstantBufferView> m_NullCbv;
};

struct ShaderResourceView : public ResourceView {
	virtual void InitAsTex2DOrCubemap(Mox::TextureResource& InTexture) = 0;

	static ShaderResourceView* GetNullTex2D() { return m_NullTex2DSrv.get(); }
	static ShaderResourceView* GetNullCube() { return m_NullCubeSrv.get(); }

protected:
	ShaderResourceView(Mox::Resource& InResource) { }

	ShaderResourceView() { }

	// Derived platform-specific class will need to initialize it
	static std::unique_ptr<ShaderResourceView> m_NullTex2DSrv;
	static std::unique_ptr<ShaderResourceView> m_NullCubeSrv;
};

struct UnorderedAccessView : public ResourceView {
	UnorderedAccessView(Mox::Resource& InReferencedResource) { }
};

struct VertexBufferView : public ResourceView{
public:
	virtual void ReferenceResource(Mox::BufferResource& InVB) = 0;

	inline bool IsGpuAllocated() override { return false; };

	inline void RebuildResourceReference() override { ReferenceResource(m_ReferencedVB); };

protected:
	VertexBufferView(Mox::BufferResource& InVB) : m_ReferencedVB(InVB) {}
	Mox::BufferResource& m_ReferencedVB;
};

struct IndexBufferView : public ResourceView {
public:
	inline uint32_t GetElementsNum() const { return m_ElementsNum; }

	virtual void ReferenceResource(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum) = 0;

	inline bool IsGpuAllocated() override { return false; };

	inline void RebuildResourceReference() override { ReferenceResource(m_ReferencedIB, m_Format, m_ElementsNum); };

protected:
	IndexBufferView(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum) 
		: m_ReferencedIB(InIB), m_Format(InFormat), m_ElementsNum(InElementsNum) { }
	Mox::BufferResource& m_ReferencedIB;
	Mox::BUFFER_FORMAT m_Format;
	uint32_t m_ElementsNum;
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

// Used to describe a texture subresource data on CPU
struct TexDataInfo
{
	const void* m_Location;
	uint32_t m_RowSize;
	uint32_t m_TotalSize;
	uint16_t m_MipLevel;
	uint16_t m_SliceIndex;
};

class TextureResource {

public:
	TextureResource(const Mox::TextureDesc& InDesc, 
		Mox::Resource& InOwnerResource,	size_t InAllocationOffset, size_t InSize);

	size_t GetGPUSize() const { return m_Size; };
	size_t GetWidth() const { return m_Width; }
	size_t GetHeight() const { return m_Height; }
	BUFFER_FORMAT GetFormat() { return m_TexelFormat; }
	TEXTURE_TYPE GetType() const { return m_Type; }
	uint16_t GetMipLevelsNum() const { return m_MipLevelsNum; }
	uint16_t GetArraySize() const { return m_ArraySize; }

	Mox::Resource& GetOwnerResource() const { return m_OwnerResource; }

	Mox::ShaderResourceView* GetView() const { return m_DefaultView; }

protected:
	Mox::Resource& m_OwnerResource;
	Mox::GPU_V_ADDRESS m_GpuPtr;
	size_t			m_Size;
	size_t          m_Width;
	size_t          m_Height;     // Should be 1 for 1D textures
	uint16_t        m_ArraySize;  // For cubemap, this is a multiple of 6
	uint16_t        m_MipLevelsNum;
	uint16_t        m_PlanesNum;
	BUFFER_FORMAT   m_TexelFormat;
	TEXTURE_TYPE    m_Type;

	Mox::ShaderResourceView* m_DefaultView;
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
