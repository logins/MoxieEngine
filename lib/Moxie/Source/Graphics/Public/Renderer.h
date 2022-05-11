/*
 Renderer.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Renderer_h__
#define Renderer_h__

#include "Async.h"
#include "ContextView.h"
#include "MoxRenderProxy.h"

namespace Mox {

	class Window;
	class CommandQueue;
	class GraphicsAllocatorBase;
	class RenderPass;
	/*
	Handles all the rendering logic for the engine
	*/
	class RenderThread : public Mox::SystemThread {

	public:
		RenderThread();

		virtual void Run() override;

		inline uint64_t GetCurrentRenderFrame() const { return m_CurrentRenderFrame; }

		Mox::CommandQueue* GetCmdQueue() { return m_CmdQueue; }

		void SetMainWindow(Mox::Window* InMainWindow);


		// Graphics Passes

		void ImportIncomingRenderUpdates(Mox::FrameRenderUpdates& InOutRenderUpdates);

	private:

		void RenderMainView();

		void RunThread();

		void OnRenderFrameStarted();

		void OnRenderFrameFinished();

		void OnFinishRunning();

		void ProcessRenderUpdates();

		// This is "camera" related data
		// TODO move it on a proper object
		std::vector<Mox::ContextView> m_ContextViews;

		Mox::Window* m_MainWindow;

		Mox::Device& m_GraphicsDevice;

		std::unique_ptr<Mox::GraphicsAllocatorBase> m_GraphicsAllocator;

		Mox::CommandQueue* m_CmdQueue;

		uint64_t m_CurrentRenderFrame = 0;

		uint64_t m_RenderFrameNumber = 0;

		// Container of render proxies which are currently involved 
		// in rendering operations by the render thread
		std::vector<Mox::RenderProxy*> m_ActiveRenderProxies;


		/*
		----- RENDER PARAMETERS UPDATES -----
		*/
	public:
		Mox::FrameRenderUpdates m_RenderUpdatesToProcess;

	};


}
#endif // Renderer_h__