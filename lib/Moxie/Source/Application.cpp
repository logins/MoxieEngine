/*
 Application.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "Application.h"
#include "MoxGeometry.h"
#include "Graphics/Public/Window.h"
#include "Graphics/Public/GraphicsAllocator.h"
#include "Graphics/Public/Device.h"
#include "Graphics/Public/CommandList.h"
#include "Simulator.h"
#include "Renderer.h"

namespace Mox
{
	std::unique_ptr<Mox::Application> Application::m_Instance; // Necessary (as standard 9.4.2.2 specifies) definition of the singleton instance

	void Application::OnMainWindowClose()
	{
		::PostQuitMessage(0); // Next message from winapi will be WM_QUIT
	}

	void Application::OnWindowPaint()
	{
		m_PaintStarted = true;
	}

	// TODO: window object belongs to the Application (so not to the simulator nor the renderer).
	// We still need a way to pass changes of the window to specific system threads.
	// e.g. I change window size, and I should enqueue a change in a common ground, where then a thread locks it, and takes all the changes.

	void Application::OnWindowResize(uint32_t InNewWidth, uint32_t InNewHeight)
	{
		m_MainWindow->Resize(InNewWidth, InNewHeight);
		// Update ViewPort here since the application acts as a "frame director" here
		// TODO this needs to be passed to the renderer by message
		//m_MainWindow->SetWidthAndHeight(static_cast<float>(InNewWidth), static_cast<float>(InNewHeight));
		SetAspectRatio(InNewWidth / static_cast<float>(InNewHeight));
	}




	Application::Application()
	{

	}

	Application::~Application() = default;

	void Application::Initialize()
	{
		uint32_t mainWindowWidth = 1024, mainWindowHeight = 768;


		m_Simulator = std::make_unique<Mox::SimulatonThread>();

		m_Renderer = std::make_unique<Mox::RenderThread>();

		Mox::WindowInitInput mainWindowInput = {
		L"DX12WindowClass", L"Main Window",
		*m_Simulator->GetCmdQueue(),
		mainWindowWidth, mainWindowHeight, // Window sizes
		mainWindowWidth, mainWindowHeight, // BackBuffer sizes
		false // vsync disabled to test max fps, but you can set it here if the used monitor allows tearing to happen
		};
		m_MainWindow = &Mox::GraphicsAllocator::Get()->AllocateWindow(mainWindowInput);

		m_Simulator->SetMainWindow(m_MainWindow);

		// Wiring Window events
		m_MainWindow->OnPaintDelegate.Add<Application, &Application::OnWindowPaint>(this);

		m_MainWindow->OnResizeDelegate.Add<Application, &Application::OnWindowResize>(this);

		m_MainWindow->OnDestroyDelegate.Add<Application, &Application::OnMainWindowClose>(this);

		// Opportunity for the derived application to initialize both simulation and graphics
		// TODO later this will have to be replaced with Simulator->Init and Renderer->Init, where they will both need to trigger callbacks for opportunity to initialize content
		
		OnInitializeContent();

		m_IsInitialized = true;

	}

	void Application::Run()
	{
		if (!m_IsInitialized)
			return;

		m_MainWindow->ShowWindow();


		m_Renderer->Run();

		// Application's main loop is based on received window messages, specifically WM_PAINT will trigger Update() and Render()
		MSG windowMessage = {};
		while (windowMessage.message != WM_QUIT)
		{
			if (::PeekMessage(&windowMessage, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&windowMessage);
				::DispatchMessage(&windowMessage);
			}

			if (!CanComputeFrame())
				continue;

			m_Simulator->Update();
		}

		m_Simulator->OnFinishRunning();

		OnQuitApplication();
	}

	uint64_t Application::GetCurrentFrameNumber()
	{
		return m_Simulator->GetCpuFrameNumber();
	}

	bool Application::CanComputeFrame()
	{
		return m_PaintStarted;
	}

	Mox::Entity& Application::AddEntity()
	{
		return m_Simulator->AddEntity();
	}

	void Application::OnQuitApplication()
	{

	}

	void Application::WaitForFrameStart_SimThread()
	{
		// --- Critical Section ---
		std::unique_lock<std::mutex> simFrameLock(m_FramesMutex);
		m_SimToRenderFrameCondVar.wait(simFrameLock, [&] { return  m_DoneRenderFrameNum + 1 >= m_DoneSimFrameNum; }); // If the Render thread is behind, we will wait for it

	}

	void Application::WaitForFrameStart_RenderThread()
	{
		// --- Critical Section ---
		std::unique_lock<std::mutex> renderFrameLock(m_FramesMutex);
		m_SimToRenderFrameCondVar.wait(renderFrameLock, [&] { return m_DoneSimFrameNum > m_DoneRenderFrameNum; }); // Render thread needs to be at least 1 frame behind the sim one
	}

	void Application::NotifyFrameEnd_SimThread()
	{
		// --- Critical Section ---
		{
			std::lock_guard<std::mutex> simFrameLock(m_FramesMutex);
			m_DoneSimFrameNum++;
		}
		m_SimToRenderFrameCondVar.notify_one();
	}

	void Application::NotifyFrameEnd_RenderThread()
	{
		// --- Critical Section ---
		{
			std::lock_guard<std::mutex> renderFrameLock(m_FramesMutex);
			m_DoneRenderFrameNum++;
		}
		m_SimToRenderFrameCondVar.notify_one();
	}

	void Application::SetFov(float InFov)
	{
		char buffer[256];
		::sprintf_s(buffer, "Fov: %f\n", InFov);
		::OutputDebugStringA(buffer);

		// TODO this needs to be passed as message to the renderer
		//m_MainContextView.SetFov(InFov);
	}

	void Application::SetAspectRatio(float InAspectRatio)
	{
		// TODO this needs to be passed as message to the renderer
		//m_MainContextView.SetAspectRatio(InAspectRatio);
	}


}
