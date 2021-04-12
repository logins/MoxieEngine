/*
 TaskSystem.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "TaskSystem.h"
#include "../../Public/MoxUtils.h"

namespace Mox {


	EngineTaskSystem::EngineTaskSystem()
	{
		// We can use std::thread::hardware_concurrency(); 
		// but that will return the number of total logical threads that we can use,
		// one of which is the main thread, so for now we use and indicative value instead.
		m_WorkerThreadsNum = 8; 

		m_TaskQueues = std::vector<TaskQueue>(m_WorkerThreadsNum);
	}

	EngineTaskSystem::~EngineTaskSystem()
	{
		for (TaskQueue& currentQueue : m_TaskQueues)
			currentQueue.SignalWorkDone();

		for (std::thread& currentThread : m_WorkerThreads)
			currentThread.join(); // TODO what happens when we call join on a thread which is executing an infinite loop??
	}

	void EngineTaskSystem::RunSystem()
	{
		for (uint32_t i = 0; i < m_WorkerThreadsNum; i++)
		{
			m_WorkerThreads.emplace_back([&, i]
				{
					RunThread(i);
				});

		}
	}

	void EngineTaskSystem::RunThread(uint32_t InThreadId)
	{
		DebugPrint("Thread " << InThreadId << "Started Execution!");

		while (true)
		{
			// Extract a task for the queue and execute it
			std::function<void()> functionToExecute;

			// If the queue corresponding to the calling thread is busy, it will try to task steal on the other queues in a circular manner.
			for (uint32_t i = 0; i < m_WorkerThreadsNum; ++i)
			{
				if (m_TaskQueues[(InThreadId + i) % m_WorkerThreadsNum].TryPop(functionToExecute))
					break; // An available queue to pop was found, so we can now execute the function
			}

			// If we did not manage to acquire the lock and pop a task from any queue, 
			// try waiting for the current thread queue to be available to and pop a task.
			// If also this fails, exit the thread loop.								// TODO is this an optimal behavior? What happens if we later want to push more stuff for this thread instead?
			if(!functionToExecute && m_TaskQueues[InThreadId].BlockingPop(functionToExecute))
				break;

			functionToExecute();
		}

		DebugPrint("Thread " << InThreadId << "Finished Execution!");

	}

	bool EngineTaskSystem::TaskQueue::TryPop(std::function<void()>& OutPoppedTask)
	{
		// ----- CRITICAL SECTION -----
		// Try to acquire the queue lock
		std::unique_lock<std::mutex> mutexLock{ m_QueueMutex, std::try_to_lock };

		if (!mutexLock || m_AsyncTaskQueue.empty()) // Note: when we check if queue empty it means we already managed to acquire the lock!
			return false;

		OutPoppedTask = std::move(m_AsyncTaskQueue.front());
		m_AsyncTaskQueue.pop();

		return true;
	}

	bool EngineTaskSystem::TaskQueue::BlockingPop(std::function<void()>& OutPoppedTask)
	{
			std::unique_lock<std::mutex> mutexLock{ m_QueueMutex };

			// If a blocking pop is ordered with an empty queue, the thread gets put to sleep,
			// waiting for the condition variable to be signaled
			while (m_AsyncTaskQueue.empty() && !m_IsWorkDone)
				m_IsReadyCondition.wait(mutexLock);

			if (m_AsyncTaskQueue.empty()) // If queue empty and work is done, return false
				return false;

			OutPoppedTask = std::move(m_AsyncTaskQueue.front());
			m_AsyncTaskQueue.pop();

			return true;
	}

	void EngineTaskSystem::TaskQueue::SignalWorkDone()
	{
		{	// ----- CRITICAL SECTION -----
			std::unique_lock<std::mutex> doneLock{ m_QueueMutex };
			m_IsWorkDone = true;
		}
		m_IsReadyCondition.notify_all();
	}

}
