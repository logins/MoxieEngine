/*
 GraphicsUtils.cpp

 First DX12 Renderer - https://github.com/logins/FirstDX12Renderer

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "GraphicsUtils.h"

#ifdef GRAPHICS_SDK_D3D12
#include "D3D12MoxUtils.h"
#endif
#include "GraphicsAllocator.h"
#include "PipelineState.h"
#include "MoxMesh.h"

namespace Mox { 

	Mox::FrameRenderUpdates& GetSimThreadUpdatesForRenderer()
	{
		static Mox::FrameRenderUpdates g_SimThreadUpdatesForRenderer;

		return g_SimThreadUpdatesForRenderer;
	}


	void RequestBufferResourceForHolder(Mox::BufferResourceHolder& InHolder)
	{
		GetSimThreadUpdatesForRenderer().m_BufferResourceRequests.emplace_back(InHolder, 
			InHolder.GetContentType(), InHolder.GetAllocType(), InHolder.GetSize(), InHolder.GetStride());
	}

	void ReleaseResourceForBuffer(Buffer& InBuffer)
	{
		// TODO
	}

	void RequestRenderProxyForEntity(Entity& InEntity, const std::vector<struct MeshCreationInfo>& InInfo)
	{
		GetSimThreadUpdatesForRenderer().m_ProxyRequests.emplace_back(InEntity, InInfo);
	}

	void ReleaseRenderProxyForEntity(Entity& InEntity)
	{
		// TODO
	}

	void UpdateConstantBufferValue(Mox::BufferResourceHolder& InBufferHolder, const void* InData, uint32_t InSize)
	{
		if (InBufferHolder.GetAllocType() == BUFFER_ALLOC_TYPE::DYNAMIC)
		{
			GetSimThreadUpdatesForRenderer().m_DynamicBufferUpdates.emplace_back(InBufferHolder, InData, InSize);
		}
		else // STATIC
		{
			GetSimThreadUpdatesForRenderer().m_StaticBufferUpdates.emplace_back(InBufferHolder, InData, InSize);
		}
	}

	void EnableDebugLayer()
	{
		return Mox::EnableDebugLayer_Internal();
	}

	Mox::BufferResource& AllocateDynamicBuffer(size_t InSize)
{
		return GraphicsAllocator::Get()->AllocateDynamicBuffer(InSize);
	}

	Mox::VertexBufferView& AllocateVertexBufferView(Mox::BufferResource& InVBResource)
{
		return GraphicsAllocator::Get()->AllocateVertexBufferView(InVBResource);
	}

	Mox::IndexBufferView& AllocateIndexBufferView(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum)
{
		return GraphicsAllocator::Get()->AllocateIndexBufferView(InIB, InFormat, InElementsNum);
	}

	Mox::ConstantBufferView& AllocateConstantBufferView(Mox::BufferResource& InResource)
{
		return GraphicsAllocator::Get()->AllocateConstantBufferView(InResource);
	}

	Mox::Shader& AllocateShader(wchar_t const* InShaderPath)
	{
		return GraphicsAllocator::Get()->AllocateShader(InShaderPath);
	}

	Mox::PipelineState& AllocatePipelineState()
{
		return GraphicsAllocator::Get()->AllocatePipelineState();
	}

	std::unique_ptr<Mox::Rect> AllocateRect(int32_t InLeft, int32_t InTop, int32_t InRight, int32_t InBottom)
	{
		return std::make_unique<Mox::D3D12Rect>(InLeft, InTop, InRight, InBottom);
	}

	std::unique_ptr<Mox::ViewPort> AllocateViewport(float InTopLeftX, float InTopLeftY, float InWidth, float InHeight)
	{
		return std::make_unique<Mox::D3D12ViewPort>(InTopLeftX, InTopLeftY, InWidth, InHeight);
	}


	RenderProxyRequest::RenderProxyRequest(Mox::Entity& InEntity, const std::vector<struct MeshCreationInfo>& InMeshInfo) 
		: m_TargetEntity(&InEntity), m_MeshInfos(InMeshInfo)
	{

	}

}