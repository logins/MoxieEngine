/*
 GraphicsTypes.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#include "GraphicsTypes.h"
#include "GraphicsAllocator.h"
#include "../Math/Public/MoxMath.h"

namespace Mox {

	VertexBuffer::VertexBuffer(Mox::CommandList& InCmdList, const void* InData, uint32_t InStride, uint32_t InSize)
		: m_Stride(InStride), m_BufferResource(Mox::GraphicsAllocator::Get()->AllocateBufferCommittedResource(InCmdList, InData, InSize))
	{
		m_DefaultView = &Mox::AllocateVertexBufferView(*this);
	}

	IndexBuffer::IndexBuffer(Mox::CommandList& InCmdList, const void* InData, int32_t InSize, int32_t InElementsNum)
		: m_BufferResource(Mox::GraphicsAllocator::Get()->AllocateBufferCommittedResource(InCmdList, InData, InSize)),
		m_ElementsNum(InElementsNum)
	{
		m_DefaultView = &Mox::AllocateIndexBufferView(*this, Mox::BUFFER_FORMAT::R16_UINT); // Single channel 16 bits, because WORD = unsigned short = 2 bytes = 16 bits
	}

	BufferResource::BufferResource(Mox::BUFFER_TYPE InBufType, Mox::Resource& InResource) 
		: m_OwningResource(InResource), m_Size(InResource.GetSize()), m_BufferType(InBufType), m_CpuPtr(InResource.GetData()), 
		m_GpuPtr(InResource.GetGpuData())
	{
		m_LocalData.resize(m_Size);
		m_DefaultView = &Mox::GraphicsAllocator::Get()->AllocateConstantBufferView(*this);
	}

	BufferResource::BufferResource(Mox::BUFFER_TYPE InBufType, Mox::Resource& InResource, void* InCpuPtr, Mox::GPU_V_ADDRESS InGpuPtr, uint32_t InSize) 
		: m_OwningResource(InResource), m_Size(InSize), m_BufferType(InBufType), m_CpuPtr(InCpuPtr), m_GpuPtr(InGpuPtr)
	{
		m_LocalData.resize(m_Size);
		m_DefaultView = &Mox::GraphicsAllocator::Get()->AllocateConstantBufferView(*this);
	}

	void BufferResource::SetData(const void* InData, int32_t InSize)
	{
		memcpy(m_LocalData.data(), InData, InSize);
	}


	void BufferResource::CopyLocalDataToLocation(void* InCpuPtr, GPU_V_ADDRESS InGpuPtr)
	{
		m_CpuPtr = InCpuPtr; m_GpuPtr = InGpuPtr;

		// TODO test
		//Mox::Matrix4f mvpValue;
		//mvpValue.data()[0] = 1.81070542;
		//mvpValue.data()[1] = 0.00000000;
		//mvpValue.data()[2] = 0.00000000;
		//mvpValue.data()[3] = 0.00000000;
		//mvpValue.data()[4] = 0.00000000;
		//mvpValue.data()[5] = 2.41421342;
		//mvpValue.data()[6] = 0.00000000;
		//mvpValue.data()[7] = 0.00000000;
		//mvpValue.data()[8] = 0.00000000;
		//mvpValue.data()[9] = 0.00000000;
		//mvpValue.data()[10] = 1.00100100;
		//mvpValue.data()[11] = 1.00000000;
		//mvpValue.data()[12] = 0.00000000;
		//mvpValue.data()[13] = 0.00000000;
		//mvpValue.data()[14] = 9.90990925;
		//mvpValue.data()[15] = 10.0000000;
		//memcpy(m_LocalData.data(), mvpValue.data(), 16);

		memcpy(InCpuPtr, m_LocalData.data(), m_LocalData.size());

		m_DefaultView->RebuildResourceReference();
	}

	Buffer::Buffer(Mox::BUFFER_TYPE InType, uint32_t InSize)
		: m_BufferType(InType), m_Size(InSize)
	{
		Mox::RequestResourceForBuffer(*this);
	}

	Buffer::~Buffer()
	{
		Mox::ReleaseResourceForBuffer(*this);
	}


	void Buffer::SetData(const void* InData, uint32_t InSize)
	{
		Mox::UpdateConstantBufferValue(*this, InData, InSize);
	}

}
