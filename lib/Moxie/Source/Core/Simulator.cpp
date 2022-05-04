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
		OnCpuFrameStarted();

		static std::chrono::steady_clock clock;
		auto t0 = clock.now();
		{
			CPU_MARKER_SPAN(Simulate, "Simulate %d", m_SimulationFrameNumber);

			Application::Get()->UpdateContent(m_DeltaTime);

		}
		auto t1 = clock.now();
		auto deltaTime = t1 - t0;
		m_DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(deltaTime).count() / 1000.f; // Delta Time expressed in Milliseconds: 10^-3 seconds
		t0 = t1;

		OnCpuFrameFinished();

		Application::Get()->SyncForFrameEnd_SimThread();
	}

	void SimulatonThread::OnCpuFrameStarted()
	{
		Application::Get()->SyncForFrameStart_SimThread();

	}

	void SimulatonThread::OnCpuFrameFinished()
	{
		// Pick up changes to transfer to the render thread
		Mox::GraphicsAllocator::Get()->TransferPendingBufferChanges(m_CurrentFrameRenderUpdates.m_ConstantUpdates);

		m_SimulationFrameNumber++;
	}

	void SimulatonThread::OnFinishRunning()
	{

	}

	Mox::Entity& SimulatonThread::CreateEntity(const Mox::EntityCreationInfo& InInfo)
{
		Mox::Entity& newEntity = m_WorldEntities.emplace_back(InInfo);

		// In case the new entity has a render proxy, and so something to render, 
		// the information will be enqueued to be delivered to the render thread. 
		if (newEntity.GetRenderProxy())
		{
			m_CurrentFrameRenderUpdates.m_NewProxies.push_back(m_WorldEntities.back().GetRenderProxy());
		}

		return m_WorldEntities.back();
	}

}
