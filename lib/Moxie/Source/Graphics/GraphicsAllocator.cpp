/*
 GraphicsAllocator.cpp

 First DX12 Renderer - https://github.com/logins/FirstDX12Renderer

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "GraphicsAllocator.h"
#include "Window.h"
#ifdef GRAPHICS_SDK_D3D12

#include "D3D12GraphicsAllocator.h"

#endif

namespace Mox { 

	Mox::GraphicsAllocatorBase* GraphicsAllocator::m_DefaultInstance = nullptr;

	std::unique_ptr<Mox::GraphicsAllocatorBase> GraphicsAllocator::CreateInstance()
	{
#ifdef GRAPHICS_SDK_D3D12
		return std::make_unique<Mox::D3D12GraphicsAllocator>();
#endif
	}

	void GraphicsAllocator::SetDefaultInstance(GraphicsAllocatorBase* InGraphicsAllocator)
	{
		m_DefaultInstance = InGraphicsAllocator;
	}


	GraphicsAllocatorBase::~GraphicsAllocatorBase() = default;

	GraphicsAllocatorBase::GraphicsAllocatorBase() = default;

}
