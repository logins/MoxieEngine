/*
 Renderer.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Renderer_h__
#define Renderer_h__

#include "Async.h"

namespace Mox {

	/*
	Handles all the rendering logic for the engine
	*/
	class RenderThread : public Mox::SystemThread {

	public:
		virtual void Run() override;

		inline uint64_t GetCurrentRenderFrame() const { return m_CurrentRenderFrame; }

	private:

		void RunThread();

		void OnRenderFrameStarted();

		void OnRenderFrameFinished();

		uint64_t m_CurrentRenderFrame = 0;

		std::thread m_InnerThread;
		uint64_t m_RenderFrameNumber = 0;
	};
}
#endif // Renderer_h__