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
#include "CpuProfiling.h"

namespace Mox {

	DEFINE_CPU_MARKER_SERIES(Simulate)

	SimulatonThread::SimulatonThread()
	{
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
		{
		CPU_MARKER_SPAN(Simulate, "Simulate %d", m_CpuFrameNumber);

			Application::Get()->UpdateContent(m_DeltaTime);

		}
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

	void SimulatonThread::OnCpuFrameStarted()
	{
		Application::Get()->WaitForFrameStart_SimThread();

	}

	void SimulatonThread::OnCpuFrameFinished()
	{

		Application::Get()->NotifyFrameEnd_SimThread();
	}

	void SimulatonThread::OnFinishRunning()
	{

	}

	Mox::Entity& SimulatonThread::AddEntity()
	{
		return m_WorldEntities.emplace_back();
	}


}
