/*
 Simulatior.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Simulatior_h__
#define Simulatior_h__

#include "Async.h"
#include "ContextView.h"
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

		Mox::CommandQueue* GetCmdQueue() { return m_CmdQueue; } // TODO this needs to be moved to Renderer when it will be created

		uint64_t GetCpuFrameNumber() { return m_CpuFrameNumber; }

		void SetMainWindow(Mox::Window* InMainWindow); // TODO move this to renderer

	private:

		void RenderMainView();

		float m_DeltaTime = 0.f;

		void OnCpuFrameStarted();

		void OnCpuFrameFinished();

		// This is "camera" related data
		// TODO move it on a proper object
		std::vector<Mox::ContextView> m_ContextViews;

		// TODO this is eventually going to the render thread
		Mox::Window* m_MainWindow;

		Mox::Device& m_GraphicsDevice;

		std::unique_ptr<Mox::GraphicsAllocatorBase> m_GraphicsAllocator;

		Mox::CommandQueue* m_CmdQueue;

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