/*
 Simulator.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "Simulator.h"
#include "Device.h"
#include "CommandList.h"
#include "Window.h"
#include "Application.h"
#include "CommandQueue.h"
#include "GraphicsAllocator.h"

namespace Mox {

	SimulatonThread::SimulatonThread()
		: m_GraphicsDevice(Mox::GetDevice())
	{
		m_GraphicsAllocator = GraphicsAllocator::CreateInstance();

		// Application has now control over the graphics allocator default instance
		GraphicsAllocator::SetDefaultInstance(m_GraphicsAllocator.get());

		Mox::GraphicsAllocator::Get()->Initialize();

		// Create Command Queue
		m_CmdQueue = &Mox::GraphicsAllocator::Get()->AllocateCommandQueue(m_GraphicsDevice, Mox::COMMAND_LIST_TYPE::COMMAND_LIST_TYPE_DIRECT);
	}

	void SimulatonThread::Run()
	{
		Update();

		OnFinishRunning();
	}

	void SimulatonThread::Update()
	{


		static double elapsedSeconds = 0;
		static uint64_t frameNumberPerSecond = 0;
		static std::chrono::high_resolution_clock clock;
		auto t0 = clock.now();

		OnCpuFrameStarted();

		Application::Get()->UpdateContent(m_DeltaTime);

		RenderMainView();

		// Frame on CPU side finished computing, send the notice
		OnCpuFrameFinished();

		m_CpuFrameNumber++;
		frameNumberPerSecond++;

		auto t1 = clock.now();
		auto deltaTime = t1 - t0;

		m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(deltaTime).count() / 1000.f; // Delta Time expressed in Milliseconds: 10^-3 seconds

		t0 = t1;
		elapsedSeconds += deltaTime.count() * 1e-9; // Conversion from nanoseconds into seconds

		if (elapsedSeconds > 1.0)
		{
			char buffer[500]; auto fps = frameNumberPerSecond / elapsedSeconds;
			sprintf_s(buffer, 500, "Average FPS: %f\n", fps);
			OutputDebugStringA(buffer);

			frameNumberPerSecond = 0;
			elapsedSeconds = .0f;
		}
	}

	void SimulatonThread::RenderMainView()
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



	void SimulatonThread::OnCpuFrameStarted()
	{
		// Checking if we are too far in frame computation compared to the GPU work.
		// If it is the case, wait for completion
		const int64_t framesToWaitNum = m_CmdQueue->ComputeFramesInFlightNum() - Application::GetMaxConcurrentFramesNum() + 1; // +1 because we need space for the current frame
		if (framesToWaitNum > 0)
		{
			m_CmdQueue->WaitForQueuedFramesOnGpu(framesToWaitNum);
		}

		// Trigger all the begin CPU frame mechanics
		m_CmdQueue->OnCpuFrameStarted();

		Mox::GraphicsAllocator::Get()->OnNewFrameStarted();
	}

	void SimulatonThread::OnCpuFrameFinished()
	{
		m_CmdQueue->OnCpuFrameFinished();
	}

	void SimulatonThread::OnFinishRunning()
	{
		// Finish all the render commands currently in flight
		m_CmdQueue->Flush();

		// Release all the allocated graphics resources
		m_GraphicsAllocator.reset();
	}

	Mox::Entity& SimulatonThread::AddEntity()
	{
		return m_WorldEntities.emplace_back();
	}

	void SimulatonThread::SetMainWindow(Mox::Window* InMainWindow)
	{
		m_MainWindow = InMainWindow;

		m_ContextViews.emplace_back(0.1f, 100.f, 0.7853981634f, m_MainWindow->GetFrameWidth(), m_MainWindow->GetFrameHeight());

	}

}
