/*
 Renderer.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "Renderer.h"
#include "Application.h"
#include "Graphics/Public/Window.h"
#include "Graphics/Public/GraphicsAllocator.h"
#include "Graphics/Public/Device.h"
#include "Graphics/Public/CommandList.h"
#include "CpuProfiling.h"

namespace Mox {

DEFINE_CPU_MARKER_SERIES(Render)

RenderThread::RenderThread()
	: m_GraphicsDevice(Mox::GetDevice())
{

	m_GraphicsAllocator = GraphicsAllocator::CreateInstance();

	// Application has now control over the graphics allocator default instance
	GraphicsAllocator::SetDefaultInstance(m_GraphicsAllocator.get());

	Mox::GraphicsAllocator::Get()->Initialize();

	// Create Command Queue
	m_CmdQueue = &Mox::GraphicsAllocator::Get()->AllocateCommandQueue(m_GraphicsDevice, Mox::COMMAND_LIST_TYPE::COMMAND_LIST_TYPE_DIRECT);
}

void RenderThread::Run()
{
	m_InnerThread = std::thread([&] { RunThread(); });
}

void RenderThread::RenderMainView()
{
	Mox::Resource& backBuffer = m_MainWindow->GetCurrentBackBuffer();

	Mox::CommandList& cmdList = m_CmdQueue->GetAvailableCommandList();

	// Clear render target and depth stencil
	{
		// Transitioning current backbuffer resource to render target state
		// We can be sure that the previous state was present because in this application all the render targets
		// are first filled and then presented to the main window repetitevely.
		cmdList.ResourceBarrier(backBuffer, RESOURCE_STATE::PRESENT, RESOURCE_STATE::RENDER_TARGET);

		float clearColor[] = { .4f, .6f, .9f, 1.f };
		cmdList.ClearRTV(m_MainWindow->GetCurrentRTVDescriptorHandle(), clearColor);

		// Note: Clearing Render Target and Depth Stencil is a good practice, but in this case is also essential.
		// Without clearing the DepthStencilView, the rasterizer would not be able to use it!!
		cmdList.ClearDepth(m_MainWindow->GetCurrentDSVDescriptorHandle());
	}

	ContextView& mainView = m_ContextViews.front();

	// Set Viewport, Scissor Rect from the main view and back buffer from the window swapchain
	{
		cmdList.SetViewportAndScissorRect(*mainView.m_Viewport, *mainView.m_ScissorRect);

		// TODO: for now, we are directly writing into a backbuffer from the swapchain, but in a real engine scenario,
		// we would first have an initial render target beforehand where we write anything we want (e.g. multiple context views)
		// and then copy the content to the swapchain's render target.
		cmdList.SetRenderTargetFromWindow(*m_MainWindow);
	}

	Application::Get()->RenderMainView(cmdList, mainView); // Derived classes will call this to render their application content

	// Execute command list and present current render target from the main window
	{
		cmdList.ResourceBarrier(backBuffer, RESOURCE_STATE::RENDER_TARGET, RESOURCE_STATE::PRESENT);

		// Mandatory for the command list to close before getting executed by the command queue
		m_CmdQueue->ExecuteCmdList(cmdList);

		m_MainWindow->Present();

	}
}

void RenderThread::RunThread()
{
	while (true)
	{
		OnRenderFrameStarted();

		static std::chrono::high_resolution_clock clock;
		auto t0 = clock.now();
		{
			CPU_MARKER_SPAN(Render, "Render %d", m_RenderFrameNumber);

			RenderMainView();
		}
		auto t1 = clock.now();
		auto deltaTime = t1 - t0;
		m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(deltaTime).count() / 1000.f; // Delta Time expressed in Milliseconds: 10^-3 seconds
		t0 = t1;

		OnRenderFrameFinished();
	}
}

void RenderThread::OnRenderFrameStarted()
{
	// Stall up until sim thread completed the next frame
	Application::Get()->WaitForFrameStart_RenderThread();

	// Checking if we are too far in frame computation compared to the GPU work.
	// If it is the case, wait for completion
	const int64_t framesToWaitNum = m_CmdQueue->ComputeFramesInFlightNum() - Application::GetMaxGpuConcurrentFramesNum();
	if (framesToWaitNum > 0)
	{
		m_CmdQueue->WaitForGpuFrames(framesToWaitNum);
	}

	// Trigger all the begin CPU frame mechanics
	m_CmdQueue->OnRenderFrameStarted();

	Mox::GraphicsAllocator::Get()->OnNewFrameStarted();

}

void RenderThread::OnRenderFrameFinished()
{
	m_CmdQueue->OnRenderFrameFinished();

	m_RenderFrameNumber++;

	Application::Get()->NotifyFrameEnd_RenderThread();
}

void RenderThread::SetMainWindow(Mox::Window* InMainWindow)
{
	m_MainWindow = InMainWindow;

	m_ContextViews.emplace_back(0.1f, 100.f, 0.7853981634f, m_MainWindow->GetFrameWidth(), m_MainWindow->GetFrameHeight());

}

void RenderThread::OnFinishRunning() // TODO need to call this!
{
	// Finish all the render commands currently in flight
	m_CmdQueue->Flush();

	// Release all the allocated graphics resources
	m_GraphicsAllocator.reset();
}

}
