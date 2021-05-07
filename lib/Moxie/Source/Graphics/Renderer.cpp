/*
 Renderer.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "Renderer.h"
#include "Application.h"
#include "CpuProfiling.h"

namespace Mox {

DEFINE_CPU_MARKER_SERIES(Render)

void RenderThread::Run()
{
	m_InnerThread = std::thread([&] { RunThread(); });
}

void RenderThread::RunThread()
{
	while (true)
	{

		OnRenderFrameStarted();

		{
			CPU_MARKER_SPAN(Render, "Render %d", m_RenderFrameNumber);

			// TODO wait some time
			auto a = std::chrono::steady_clock::now();
			auto timeToWait = std::chrono::milliseconds(1);

			while ((std::chrono::steady_clock::now() - a) < timeToWait)
				continue;
		}

		OnRenderFrameFinished();
	}
}

void RenderThread::OnRenderFrameStarted()
{
	// Stall up until sim thread completed the next frame
	Application::Get()->WaitForFrameStart_RenderThread();
}

void RenderThread::OnRenderFrameFinished()
{
	m_RenderFrameNumber++;

	Application::Get()->NotifyFrameEnd_RenderThread();
}

}
