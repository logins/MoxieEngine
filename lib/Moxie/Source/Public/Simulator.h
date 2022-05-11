/*
 Simulatior.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Simulatior_h__
#define Simulatior_h__

#include "Async.h"
#include "MoxEntity.h"
#include "MoxRenderProxy.h"

namespace Mox {

	struct SimThreadInitData;
	class CommandQueue;
	class CommandList;
	class Device;
	class GraphicsAllocatorBase;
	class Window;
	struct FrameRenderUpdates;

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

		Mox::Entity& CreateEntity(const Mox::EntityCreationInfo& InInfo);

		uint64_t GetCpuFrameNumber() { return m_SimulationFrameNumber; }

		// Called from Application to transfer object parameters changes to the Render thread



	private:

		void OnCpuFrameStarted();

		void OnCpuFrameFinished();



		std::vector<Mox::Entity> m_WorldEntities;


		uint64_t m_SimulationFrameNumber = 1;


	};

	// Used to initialize the simulator
	struct SimThreadInitData 
	{
		Mox::Window* m_MainWindow;
	};

}
#endif // Simulatior_h__