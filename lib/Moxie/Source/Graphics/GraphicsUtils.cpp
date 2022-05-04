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
#include "Public/PipelineState.h"

namespace Mox { 

#ifdef GRAPHICS_SDK_D3D12

	void EnableDebugLayer()
	{
		return Mox::EnableDebugLayer_Internal();
	}

	Mox::Buffer& AllocateDynamicBuffer(size_t InSize)
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

	Mox::ConstantBufferView& AllocateConstantBufferView(Mox::Buffer& InResource)
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


#endif

}