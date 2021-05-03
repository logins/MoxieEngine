/*
 ThreadsExample.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
#include "TaskSystem.h"
#include "MoxUtils.h"
#include "CpuProfiling.h"

int main()
{
	// Mutithreading test
	Mox::EngineTaskSystem taskSystem;

	// Adding tasks
	const uint32_t numTasks = 1000;
	static const uint32_t maxSingleTaskSeconds = 4;

	for (size_t i = 1; i < numTasks; i++)
	{
		taskSystem.Enqueue([i] {

			CPU_MARKER_SPAN("Task %d", i)

			auto a = std::chrono::steady_clock::now();
			auto timeToWait = std::chrono::seconds((std::rand() % maxSingleTaskSeconds) + 1);

			while ((std::chrono::steady_clock::now() - a) < timeToWait)
				continue;

			DebugPrint("Work done for " << i );

			});
	}

	taskSystem.RunSystem();

	// Note: the taskSystem object will go out of scope, call the destructor,
	// and this last will call Join on the worker threads, effectively stalling the main thread
	// up until they will exit their running loop, so that when taks to execute are finished.

}