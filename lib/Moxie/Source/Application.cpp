/*
 Application.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "Application.h"
#include "MoxGeometry.h"
#include "Simulator.h"
#include "Renderer.h"
#include "Graphics/Public/Window.h"
#include "Graphics/Public/GraphicsAllocator.h"

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
		: m_DoneSimFrameNum(0), m_DoneRenderFrameNum(0)
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
		* m_Renderer->GetCmdQueue(),
		mainWindowWidth, mainWindowHeight, // Window sizes
		mainWindowWidth, mainWindowHeight, // BackBuffer sizes
		false // vsync disabled to test max fps, but you can set it here if the used monitor allows tearing to happen
		};
		m_MainWindow = &Mox::GraphicsAllocator::Get()->AllocateWindow(mainWindowInput); // TODO move the window outside graphics allocator

		m_Renderer->SetMainWindow(m_MainWindow);

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
			static double elapsedSeconds = 0;
			static uint32_t perSecondFrameNum = 0;
			static std::chrono::steady_clock clock;
			auto t0 = clock.now();

			if (::PeekMessage(&windowMessage, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&windowMessage);
				::DispatchMessage(&windowMessage);
			}

			if (!CanComputeFrame())
				continue;

			m_Simulator->Update();

			auto t1 = clock.now();
			elapsedSeconds += std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count(); // fractional seconds
			perSecondFrameNum++;
			t0 = t1;

			if (elapsedSeconds > 1.0f)
			{
				char buffer[500];
				sprintf_s(buffer, 500, "Average FPS: %.0f (Simulation %.2fms, Render %.2fms)\n", 
					perSecondFrameNum / elapsedSeconds, m_SimulationFrameTime, m_RenderFrameTime);
				OutputDebugStringA(buffer);

				perSecondFrameNum = 0;
				elapsedSeconds = .0f;
			}
		}

		OrderThreadsTermination();
		// Wait for the render thread to terminate since it will release resources
		m_Renderer->Join();
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

	Mox::Entity& Application::AddEntity(const Mox::EntityCreationInfo& InInfo)
	{
		return m_Simulator->CreateEntity(InInfo);
	}

	void Application::OnQuitApplication()
	{

	}

	bool Application::SyncForFrameStart_SimThread()
	{
		// --- Critical Section ---
		std::unique_lock<std::mutex> simFrameLock(m_FramesMutex);
		if (m_IsTerminating)
			return false;
		
		m_SimToRenderFrameCondVar.wait(simFrameLock, [&] { return  m_DoneRenderFrameNum + 1 >= m_DoneSimFrameNum; }); // If the Render thread is behind, we will wait for it

		// Syncing data from Simulator to Application
		m_SimulationFrameTime = m_Simulator->GetCurrentFrameTime();

		return true;
	}

	bool Application::SyncForFrameStart_RenderThread()
	{
		// --- Critical Section ---
		std::unique_lock<std::mutex> renderFrameLock(m_FramesMutex);
		if (m_IsTerminating)
			return false;
		
		m_SimToRenderFrameCondVar.wait(renderFrameLock, [&] { return m_DoneSimFrameNum > m_DoneRenderFrameNum; }); // Render thread needs to be at least 1 frame behind the sim one
	
		// Syncing data from Renderer to Application
		m_RenderFrameTime = m_Renderer->GetCurrentFrameTime();

		// The render thread retrieves and process the render updates to the render proxies
		// 
		// TODO find a better way to move updates from the simulation to the render thread

		m_Renderer->m_RenderUpdatesToProcess = m_StagedRenderUpdates;
		m_StagedRenderUpdates = Mox::FrameRenderUpdates();

		return true;
	}

// Moves Src vector at the end of Dst
#define MOVE_VEC(Dst,Src) Dst.insert(Dst.end(),std::make_move_iterator(Src.begin()),std::make_move_iterator(Src.end()));

	void Application::SyncForFrameEnd_SimThread()
	{
		// --- Critical Section ---
		{
			std::lock_guard<std::mutex> simFrameLock(m_FramesMutex);

			// Stage changes requested from the simulation
			const auto& newUpdates = Mox::GetSimThreadUpdatesForRenderer();
			MOVE_VEC(m_StagedRenderUpdates.m_BufferResourceRequests, newUpdates.m_BufferResourceRequests)
			MOVE_VEC(m_StagedRenderUpdates.m_ConstantUpdates, newUpdates.m_ConstantUpdates)
			MOVE_VEC(m_StagedRenderUpdates.m_ProxyRequests, newUpdates.m_ProxyRequests)

			Mox::GetSimThreadUpdatesForRenderer() = Mox::FrameRenderUpdates();

			m_DoneSimFrameNum++;
		}
		m_SimToRenderFrameCondVar.notify_one();
	}

	void Application::SyncForFrameEnd_RenderThread()
	{
		// --- Critical Section ---
		{
			std::lock_guard<std::mutex> renderFrameLock(m_FramesMutex);
			m_DoneRenderFrameNum++;
		}
		m_SimToRenderFrameCondVar.notify_one();
	}

	void Application::OrderThreadsTermination()
	{
		// --- Critical Section ---
		std::lock_guard<std::mutex> simFrameLock(m_FramesMutex);
		m_IsTerminating = true;
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
