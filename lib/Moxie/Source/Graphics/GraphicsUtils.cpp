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


	void RequestResourceForBuffer(Buffer& InBuffer)
	{
		GetSimThreadUpdatesForRenderer().m_BufferResourceRequests.emplace_back(InBuffer, InBuffer.GetType(), InBuffer.GetSize());
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

	void UpdateConstantBufferValue(Buffer& InBuffer, const void* InData, uint32_t InSize)
	{
		GetSimThreadUpdatesForRenderer().m_ConstantUpdates.emplace_back(InBuffer, InData, InSize);
	}

	void EnableDebugLayer()
	{
		return Mox::EnableDebugLayer_Internal();
	}

	Mox::BufferResource& AllocateDynamicBuffer(size_t InSize)
{
		return GraphicsAllocator::Get()->AllocateDynamicBuffer(InSize);
	}

	Mox::VertexBufferView& AllocateVertexBufferView(Mox::VertexBuffer& InVB)
{
		return GraphicsAllocator::Get()->AllocateVertexBufferView(InVB);
	}

	Mox::IndexBufferView& AllocateIndexBufferView(Mox::IndexBuffer& InIB, Mox::BUFFER_FORMAT InFormat)
{
		return GraphicsAllocator::Get()->AllocateIndexBufferView(InIB, InFormat);
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