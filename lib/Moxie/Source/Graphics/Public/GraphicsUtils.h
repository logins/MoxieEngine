/*
 GraphicsUtils.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef GraphicsUtils_h__
#define GraphicsUtils_h__

#include "GraphicsTypes.h"
#include <memory> // for std::unique_ptr

// Note: MOX_ROOT_PATH should be coming from CMake
#define Q(x) L#x
#define LQUOTE(x) Q(x)
#define MOX_SHADERS_PATH(NAME) LQUOTE(MOX_ROOT_PATH/Shaders/NAME)

// TODO move this in general Utils (not graphics specific)
// this has taken inspiration from DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) defined in winnt.h
#define DEFINE_OPERATORS_FOR_ENUM(ENUMTYPE) \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) {return static_cast<ENUMTYPE>((int)a | b);} \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b){return a = static_cast<ENUMTYPE>((int)a | b);} \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b){return static_cast<ENUMTYPE>((int)a & b);} \
inline ENUMTYPE operator &= (ENUMTYPE &a, ENUMTYPE b){return a = static_cast<ENUMTYPE>((int)a & b);}


namespace Mox { 

	class PipelineState;
	class Device;
	struct BufferResource;
	class VertexBuffer;
	struct VertexBufferView;
	class IndexBuffer;
	struct IndexBufferView;
	class Entity;

	// Stores an update for a constant buffer to be then applied by the render thread
	struct BufferResourceUpdate
	{
		BufferResourceUpdate(Mox::BufferResourceHolder& InBufferHolder, const void* InData, uint32_t InSize)
			: m_BufResHolder(&InBufferHolder), m_UpdateData(std::vector<std::byte>(InSize))
		{
			memcpy(m_UpdateData.data(), InData, InSize);
		}
		Mox::BufferResourceHolder* m_BufResHolder;
		std::vector<std::byte> m_UpdateData;
		// Note: This will have to be executed by the render thread
		inline void ApplyUpdate()
		{
			m_BufResHolder->GetResource()->SetData(m_UpdateData.data(), m_UpdateData.size());
		}
	};

	struct BufferResourceRequest
	{
		Mox::BufferResourceHolder* m_TargetBufferHolder;
		Mox::RES_CONTENT_TYPE m_ContentType;
		Mox::BUFFER_ALLOC_TYPE m_AllocType;
		uint32_t m_Stride;
		uint32_t m_AllocationSize;

		BufferResourceRequest(Mox::BufferResourceHolder& InBufferHolder, Mox::RES_CONTENT_TYPE InContentType, 
			Mox::BUFFER_ALLOC_TYPE InAllocType, uint32_t InSize, uint32_t InStride)
			: m_TargetBufferHolder(&InBufferHolder), m_ContentType(InContentType), 
			m_AllocType(InAllocType), m_Stride(InStride), m_AllocationSize(InSize) { }

	};

	// When the render proxy will be created for the target entity, 
	// the render resources for the bound meshes will be created as well
	struct RenderProxyRequest
	{
		Mox::Entity* m_TargetEntity;

		RenderProxyRequest(Mox::Entity& InEntity);

	};

	// Definition for the Shader Parameter Hash value type
	using SpHash = uint64_t;

	using MeshParamsList = std::vector<std::tuple<Mox::SpHash, Mox::ConstantBuffer*>>;

	struct DrawableCreationInfo
	{
		Mox::Entity* m_OwningEntity;
		Mox::VertexBuffer* m_VertexBuffer;
		Mox::IndexBuffer* m_IndexBuffer;
		MeshParamsList m_ShaderParameters;
	};

	// Used by the simulation thread to transfer object changes to the render thread
	struct FrameRenderUpdates
	{
		std::vector<Mox::RenderProxyRequest> m_ProxyRequests;

		std::vector<Mox::DrawableCreationInfo> m_DrawableRequests;

		std::vector<Mox::BufferResourceRequest> m_BufferResourceRequests;

		std::vector<Mox::BufferResourceUpdate> m_DynamicBufferUpdates;

		std::vector<Mox::BufferResourceUpdate> m_StaticBufferUpdates;
	};

	// Render updates are meant to be filled in the simulation thread
	// to then be picked up by the render thread during Application class sync-frame mechanics
	Mox::FrameRenderUpdates& GetSimThreadUpdatesForRenderer();

	// Stores a request of creating a buffer resource for the given buffer
	void RequestBufferResourceForHolder(Mox::BufferResourceHolder& InHolder);
	// Stores a request of releasing the buffer resource associated with the given buffer
	void ReleaseResourceForBuffer(Mox::ConstantBuffer& InBuffer);

	void RequestRenderProxyForEntity(Mox::Entity& InEntity);

	void ReleaseRenderProxyForEntity(Mox::Entity& InEntity);

	void RequestDrawable(const DrawableCreationInfo& InCreationInfo);

	void UpdateConstantBufferValue(Mox::BufferResourceHolder& InBufferHolder, const void* InData, uint32_t InSize);


	// Note: If we had another SDK to choose from, the same functions would be defined again returning objects from the other SDK

	void EnableDebugLayer();

	Mox::BufferResource& AllocateDynamicBuffer(size_t InSize);

	Mox::VertexBufferView& AllocateVertexBufferView(Mox::BufferResource& InVBResource);
	Mox::IndexBufferView& AllocateIndexBufferView(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum);
	Mox::ConstantBufferView& AllocateConstantBufferView(Mox::BufferResource& InResource);

	Mox::Shader& AllocateShader(wchar_t const* InShaderPath);

	Mox::PipelineState& AllocatePipelineState();

	std::unique_ptr<Rect> AllocateRect(int32_t InLeft, int32_t InTop, int32_t InRight, int32_t InBottom);

	std::unique_ptr<ViewPort> AllocateViewport(float InTopLeftX, float InTopLeftY, float InWidth, float InHeight);



	// ----- SHADER PARAMETER HASHING -----
// At the moment shader parameter hashing is implemented as FNV-1a from https://gist.github.com/hwei/1950649d523afd03285c
// with the difference of being the 64 bit version, which should only change the prime and the offset, more info here
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash
// This is not the fastest to compute, but it is straight forward 
// TODO later better passing to XXH64 as it's faster: http://cyan4973.github.io/xxHash/
	static const uint64_t FNV_PRIME = 1099511628211ULL;
	static const uint64_t OFFSET_BASIS = 14695981039346656037ULL;
	template <uint32_t N> // Compile time version
	static constexpr SpHash HashSpName(const char(&str)[N], uint32_t I = N)
	{
		return I == 1 ? (OFFSET_BASIS ^ str[0]) * FNV_PRIME : (HashSpName(str, I - 1) ^ str[I - 1]) * FNV_PRIME;
	}

}
#endif // GraphicsUtils_h__
