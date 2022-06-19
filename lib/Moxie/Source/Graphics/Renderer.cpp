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
#include "Features/Public/RenderPass.h"
#include "MoxDrawable.h"

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

	// ----- Setup Render Passes -----
	for (const std::unique_ptr<Mox::RenderPass>& pass : GetRenderPasses())
	{
		pass->SetupPass();
	}

}

void RenderThread::Run()
{
	m_InnerThread = std::make_unique<std::thread>([&] { RunThread(); });
}

void RenderThread::RenderMainView()
{
	Mox::Resource& backBuffer = m_MainWindow->GetCurrentBackBuffer();

	Mox::CommandList& cmdList = m_CmdQueue->GetAvailableCommandList();

	// Clear render target and depth stencil
	m_MainWindow->ClearRtAndDs(cmdList);
	ContextView& mainView = m_ContextViews.front();

	// Set Viewport, Scissor Rect from the main view and back buffer from the window swapchain
	{
		cmdList.SetViewportAndScissorRect(*mainView.m_Viewport, *mainView.m_ScissorRect);

		// TODO: for now, we are directly writing into a backbuffer from the swapchain, but in a real engine scenario,
		// we would first have an initial render target beforehand where we write anything we want (e.g. multiple context views)
		// and then copy the content to the swapchain's render target.
		cmdList.SetRenderTargetFromWindow(*m_MainWindow);
	}

	// ----- Send Draw Commands -----
	for (const std::unique_ptr<Mox::RenderPass>& pass : Mox::GetRenderPasses())
	{
		pass->SendDrawCommands(cmdList);
	}

	// Execute command list and present current render target from the main window
	{
		cmdList.ResourceBarriers(TransitionInfoVector{ 
			{&backBuffer, RESOURCE_STATE::RENDER_TARGET, RESOURCE_STATE::PRESENT} 
			});

		// Mandatory for the command list to close before getting executed by the command queue
		m_CmdQueue->ExecuteCmdList(cmdList);

		m_MainWindow->Present();

	}
}

void RenderThread::RunThread()
{
	while (true)
	{
		// Sync data with the application, if it returns false it means the application is ending and so does the render thread
		if (!Application::Get()->SyncForFrameStart_RenderThread())
		{
			OnFinishRunning();
			return;
		}

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

		Application::Get()->SyncForFrameEnd_RenderThread();
	}
}

void RenderThread::OnRenderFrameStarted()
{
	// Checking if we are too far in frame computation compared to the GPU work.
	// If it is the case, wait for completion
	const int64_t framesToWaitNum = m_CmdQueue->ComputeFramesInFlightNum() - Application::GetMaxGpuConcurrentFramesNum();
	if (framesToWaitNum > 0)
	{
		m_CmdQueue->WaitForGpuFrames(framesToWaitNum);
	}

	// Trigger all the begin CPU frame mechanics
	m_CmdQueue->OnRenderFrameStarted();


	// Update the live proxies with the given parameters
	ProcessRenderUpdates();

	Mox::GraphicsAllocator::Get()->OnNewFrameStarted();

}

void RenderThread::OnRenderFrameFinished()
{
	m_CmdQueue->OnRenderFrameFinished();

	Mox::GraphicsAllocator::Get()->OnNewFrameEnded();

	m_RenderFrameNumber++;
}

void RenderThread::SetMainWindow(Mox::Window* InMainWindow)
{
	m_MainWindow = InMainWindow;

	m_ContextViews.emplace_back(0.1f, 100.f, 0.7853981634f, m_MainWindow->GetFrameWidth(), m_MainWindow->GetFrameHeight());

}

void RenderThread::ImportIncomingRenderUpdates(Mox::FrameRenderUpdates& InOutRenderUpdates)
{
	m_RenderUpdatesToProcess = std::move(InOutRenderUpdates);

	InOutRenderUpdates = Mox::FrameRenderUpdates();
}

void RenderThread::OnFinishRunning()
{
	// Finish all the render commands currently in flight
	m_CmdQueue->Flush();

	// Release all the allocated graphics resources
	m_GraphicsAllocator.reset();
}

void RenderThread::ProcessRenderUpdates()
{
	// Create buffer resources
	for (const Mox::BufferResourceRequest& resourceRequest : m_RenderUpdatesToProcess.m_BufferResourceRequests)
	{
		GraphicsAllocator::Get()->AllocateResourceForBuffer(resourceRequest);

	}

	// Create textures
	for (const Mox::TextureResourceRequest& texRequest : m_RenderUpdatesToProcess.m_TextureResourceRequests)
	{
		GraphicsAllocator::Get()->AllocateTextureResource(texRequest);
	}

	// Create proxies
	std::vector<Mox::RenderProxy*> newProxies = GraphicsAllocator::Get()->CreateProxies(m_RenderUpdatesToProcess.m_ProxyRequests);

	// Create Drawables
	GraphicsAllocator::Get()->CreateDrawables(m_RenderUpdatesToProcess.m_DrawableRequests);

	// Handling new render proxies
	for (Mox::RenderProxy* newProxy : newProxies)
	{
		// Make every pass aware of the new proxies to be able to create relative draw commands
		for (const std::unique_ptr<Mox::RenderPass>& pass : Mox::GetRenderPasses())
		{
			pass->ProcessRenderProxy(*newProxy);
		}

		m_ActiveRenderProxies.push_back(newProxy);
	}

	// Update constant buffer values
	for (BufferResourceUpdate& constUpdate : m_RenderUpdatesToProcess.m_DynamicBufferUpdates)
	{
		constUpdate.ApplyUpdate();

	}

	if (m_RenderUpdatesToProcess.m_StaticBufferUpdates.size() > 0 || m_RenderUpdatesToProcess.m_TextureUpdates.size() > 0)
	{
		// Update static resources
		Mox::CommandList& loadContentCmdList = GetCmdQueue()->GetAvailableCommandList();

		Mox::GraphicsAllocator::Get()->UpdateStaticBufferResources(loadContentCmdList, m_RenderUpdatesToProcess.m_StaticBufferUpdates);

		Mox::GraphicsAllocator::Get()->UpdateTextureResources(loadContentCmdList, m_RenderUpdatesToProcess.m_TextureUpdates);

		GetCmdQueue()->ExecuteCmdList(loadContentCmdList);

		GetCmdQueue()->Flush();
	}

	m_RenderUpdatesToProcess = Mox::FrameRenderUpdates();
}

}
