/*
 Simulatior.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Simulatior_h__
#define Simulatior_h__

#include "Async.h"
#include "Entity.h"

namespace Mox {

	struct SimThreadInitData;
	class CommandQueue;
	class CommandList;
	class Device;
	class GraphicsAllocatorBase;
	class Window;

	/* SimulatonThread is the system that handles all the application simulation, e.g. gameplay, 
	* and to update the properties of objects in the scene.
	* 
	* It is supposed to run on the Main (starter) thread.
	*/
	class SimulatonThread : public Mox::SystemThread {

	public:

		SimulatonThread();

		virtual void Run();

		void Update();

		void OnFinishRunning();

		Mox::Entity& AddEntity();

		uint64_t GetCpuFrameNumber() { return m_CpuFrameNumber; }


	private:

		void OnCpuFrameStarted();

		void OnCpuFrameFinished();



		std::vector<Mox::Entity> m_WorldEntities;

		uint64_t m_CpuFrameNumber = 1;

	};

	// Used to initialize the simulator
	struct SimThreadInitData 
	{
		Mox::Window* m_MainWindow;
	};

}
#endif // Simulatior_h__