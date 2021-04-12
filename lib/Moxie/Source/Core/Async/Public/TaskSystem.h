/*
 TaskSystem.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef TaskSystem_h__
#define TaskSystem_h__

#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>

// This concept of task system is inspired to the implementation
// found in Vorbrodt's C++ Blog at https://vorbrodt.blog/2019/02/27/advanced-thread-pool/
// with GitHub at https://github.com/mvorbrodt/blog/blob/master/src/pool.hpp
// and a similar implementation https://github.com/xSeditx/Creature-Engine/blob/master/CreatureEngine/Core/Threading/Threadpool.h
// and the presentation of Sean Parent https://www.youtube.com/watch?v=zULU6Hhp42w

// Task stealing queue concept is that if a thread finds a queue to be busy, because another thread is accessing it or the queue is empty,
// then the thread will look to steal a job from another queue.

namespace Mox {

	// Abstact class that acts as interface for a task system implementation
	class TaskSystem
	{
	/*	template<typename TFunc>
		void Enqueue(TFunc&& InFunction) = 0;*/

	};

	// Used by the Moxie Engine main task system
	class EngineTaskSystem : public TaskSystem
	{
	public:
		// Creates a set of worker threads
		EngineTaskSystem();

		// Shuts down the system and joins the worker threads
		~EngineTaskSystem();

		void RunSystem();

		template<typename TFunc>
		void Enqueue(TFunc&& InFunction)
		{
			uint32_t queueIndex = m_PushQueueId++;

			for (uint32_t i = 0; i < m_WorkerThreadsNum; i++)
			{
				if (m_TaskQueues[(queueIndex + i) % m_WorkerThreadsNum].TryPush(std::forward<TFunc>(InFunction)))
					return;
			}

			// If we were not able to acquire the lock and push to any queue, we are gonna wait for the current queue to be available and then push the task
			m_TaskQueues[queueIndex % m_WorkerThreadsNum].BlockingPush(std::forward<TFunc>(InFunction));
		}

	private:

		// Runs a thread main loop
		void RunThread(uint32_t InThreadId);

		struct AsyncTask;

		struct TaskQueue;

		struct WorkerThread;

		uint32_t m_WorkerThreadsNum;
		std::vector<TaskQueue> m_TaskQueues;

		std::vector<std::thread> m_WorkerThreads;

		std::atomic<uint32_t> m_PushQueueId{ 0 };
	};

	struct WorkerThread
	{
		TaskSystem& m_TaskSystem;
	};

	struct AsyncTask
	{

	};

	
	struct EngineTaskSystem::TaskQueue
	{
		TaskQueue()
			: m_ThreadId(std::this_thread::get_id())
		{}

		void ShutDown();

		// True if managed to acquire a lock on the queue mutex and pop a task from the queue.
		bool TryPop(std::function<void()>& OutPoppedTask);

		bool BlockingPop(std::function<void()>& OutPoppedTask);

		// True if managed to acquire a lock on the queue mutex and push a task into the queue.

		template<typename TFunc>
		bool TryPush(TFunc&& InTaskToQueue)
		{
			{	// ----- CRITICAL SECTION -----
				// Try to acquire the queue lock
				std::unique_lock<std::mutex> mutexLock{ m_QueueMutex, std::try_to_lock };

				if (!mutexLock)	return false;

				m_AsyncTaskQueue.emplace(std::forward<TFunc>(InTaskToQueue));
			} // Note: here the mutexLock gets destroyed because exiting scope, and the thread exits the critical section

			m_IsReadyCondition.notify_one(); // This is for the cases of blocking pop

			return true;
		}

		template<typename TFunc>
		bool BlockingPush(TFunc&& InTaskToQueue)
		{
			{	// ----- CRITICAL SECTION -----
				std::unique_lock<std::mutex> mutexLock{ m_QueueMutex };

				m_AsyncTaskQueue.emplace(std::forward<TFunc>(InTaskToQueue));
			}

			m_IsReadyCondition.notify_one();

			return true;
		}

		void SignalWorkDone();

		std::thread::id m_ThreadId;
		std::mutex m_QueueMutex;
		std::condition_variable m_IsReadyCondition;
		std::queue<std::function<void()>> m_AsyncTaskQueue;
		bool m_IsWorkDone = false;
	};

}
#endif // TaskSystem_h__