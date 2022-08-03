/*
 GraphicsTypes.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#include "GraphicsTypes.h"
#include "GraphicsAllocator.h"
#include "MoxMath.h"
#include "MoxUtils.h"
#include "Public/MoxResourceLoader.h"

namespace Mox {

	BufferResourceHolder::~BufferResourceHolder() = default;

	void BufferResourceHolder::SetData(const void* InData, uint32_t InSize)
	{
		Mox::UpdateConstantBufferValue(*this, InData, InSize);
	}

	VertexBuffer::VertexBuffer(const void* InData, uint32_t InStride, uint32_t InSize)
		: BufferResourceHolder(Mox::RES_CONTENT_TYPE::VERTEX, Mox::BUFFER_ALLOC_TYPE::STATIC, InSize, InStride)
	{
		Mox::RequestBufferResourceForHolder(*this);

		Mox::UpdateConstantBufferValue(*this, InData, InSize);
	}

	IndexBuffer::IndexBuffer(const void* InData, uint32_t InStride, uint32_t InSize)
		: m_ElementsNum(InSize / InStride), BufferResourceHolder(Mox::RES_CONTENT_TYPE::INDEX, Mox::BUFFER_ALLOC_TYPE::STATIC, InSize, InStride)
	{
		Check(InSize % InStride == 0);
		Mox::RequestBufferResourceForHolder(*this);
		
		Mox::UpdateConstantBufferValue(*this, InData, InSize);
	}

	void BufferResource::CreateDefaultView()
	{
		switch (m_ContentType)
		{
		case RES_CONTENT_TYPE::CONSTANT:
		{
			m_DefaultView = &Mox::GraphicsAllocator::Get()->AllocateConstantBufferView(*this);
			break;
		}
		case RES_CONTENT_TYPE::VERTEX:
		{
			m_DefaultView = &Mox::AllocateVertexBufferView(*this);
			break;
		}
		case RES_CONTENT_TYPE::INDEX:
		{
			m_DefaultView = &Mox::AllocateIndexBufferView(*this, Mox::BUFFER_FORMAT::R16_UINT, m_ElementsNum); // Single channel 16 bits, because WORD = unsigned short = 2 bytes = 16 bits
			break;
		}
		}
		
	}


	BufferResource::BufferResource(Mox::RES_CONTENT_TYPE InContentType, Mox::BUFFER_ALLOC_TYPE InBufType,
		Mox::Resource& InResource, void* InCpuPtr, Mox::GPU_V_ADDRESS InGpuPtr, uint32_t InSize, uint32_t InStride)
		: m_OwningResource(InResource), m_Size(InSize), m_ContentType(InContentType), 
		m_BufferType(InBufType), m_CpuPtr(InCpuPtr), m_GpuPtr(InGpuPtr), m_Stride(InStride), m_ElementsNum(InSize/InStride)
	{
		Check(InSize % InStride == 0)

		m_LocalData.resize(m_Size); // TODO this needs to be used only with constant buffers

		CreateDefaultView();
	}

	void BufferResource::SetData(const void* InData, int32_t InSize)
	{
		memcpy(m_LocalData.data(), InData, InSize);
	}


	void BufferResource::CopyLocalDataToLocation(void* InCpuPtr, GPU_V_ADDRESS InGpuPtr)
	{
		m_CpuPtr = InCpuPtr; m_GpuPtr = InGpuPtr;

		memcpy(InCpuPtr, m_LocalData.data(), m_LocalData.size());

		m_DefaultView->RebuildResourceReference();
	}

	ConstantBuffer::ConstantBuffer(Mox::BUFFER_ALLOC_TYPE InType, uint32_t InSize)
		: BufferResourceHolder(Mox::RES_CONTENT_TYPE::CONSTANT, InType, InSize)
	{
		Mox::RequestBufferResourceForHolder(*this);
	}

	ConstantBuffer::~ConstantBuffer()
	{
		Mox::ReleaseResourceForBuffer(*this);
	}

	TextureResource::TextureResource(const Mox::TextureDesc& InDesc, 
		Mox::Resource& InOwnerResource, size_t InAllocationOffset, size_t InSize)
		: m_OwnerResource(InOwnerResource),
		m_GpuPtr(InOwnerResource.GetGpuData() + InAllocationOffset),
		m_Size(InOwnerResource.GetSize()),
		m_Width(InDesc.m_Width),
		m_Height(InDesc.m_Height),
		m_ArraySize(InDesc.m_ArraySize),
		m_MipLevelsNum(InDesc.m_MipLevelsNum),
		m_PlanesNum(InDesc.m_PlanesNum),
		m_TexelFormat(InDesc.m_TexelFormat),
		m_Type(InDesc.m_Type)
	{
		CreateDefaultView();
	}

	void TextureResource::CreateDefaultView()
	{
		// Note: This will create the view on CPU side, we will also need to upload it to GPU with a command list
		m_DefaultView = &Mox::GraphicsAllocator::Get()->AllocateShaderResourceView(*this);
	}

	Texture::Texture(TextureDesc& InDesc)
		: m_Desc(InDesc)
	{
		Mox::RequestTextureResource(*this, InDesc);

	}

	Texture::Texture(const wchar_t* InFilePath)
	{
		// Load texture data from file
		std::vector<Mox::TexDataInfo> subResInfo;
		const void* texData;
		size_t texEntireSize;

		if (Mox::ResourceLoader::Get().LoadTextureData(
			InFilePath, m_Desc, m_TextureData, texData, texEntireSize, subResInfo))
		{
			// Ask for tex resource creation
			Mox::RequestTextureResource(*this, m_Desc);

			// Update tex resource content
			UpdateContent(texData, texEntireSize, subResInfo);
		}

		
	}

	void Texture::UpdateContent(const void* InUpdateData, size_t InUpdateSize, std::vector<Mox::TexDataInfo>& InUpdateInfo)
	{
		// Note: we are assuming UpdateInfo data is stored in contiguous memory

		Mox::UpdateTextureContent(Mox::TextureResourceUpdate{
		InUpdateData,
		InUpdateSize,
		std::move(InUpdateInfo),
		this
		});
	}

	

	Mox::ConstantBufferView* ConstantBufferView::GetNull()
	{
		return m_NullCbv.get();
	}

	ConstantBufferView::ConstantBufferView() = default;

	std::unique_ptr<Mox::ConstantBufferView> ConstantBufferView::m_NullCbv;

	std::unique_ptr<Mox::ShaderResourceView> ShaderResourceView::m_NullTex2DSrv;

	std::unique_ptr<Mox::ShaderResourceView> ShaderResourceView::m_NullCubeSrv;

}
