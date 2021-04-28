/*
 Application.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Application_h__
#define Application_h__

#include "MoxUtils.h"
#include "MoxMath.h"
#include "ContextView.h"

namespace Mox {
	
		class Device;
		class Window;
		class GraphicsAllocatorBase;
		struct Rect;
		struct ViewPort;
		class CommandList;
		class SimulatonThread;
		class Entity;

	/*!
	 * \class Application
	 *
	 * \brief Represents the whole application.
	 * This is intended to be derived to implement proper functionality for each project part.
	 * Application acts as a main hub to generate most of the other objects required to run each example.
	 *
	 * \author Riccardo Loggini
	 * \date July 2020
	 */
	class Application
	{
	public:
		// Marking destructor as virtual will make destructor from derived classes to execute first
		virtual ~Application();

		template<typename T>
		static void Create()
		{
			m_Instance = std::make_unique<T>();
		}

		static Application* Get() { return m_Instance.get(); };

		void Initialize();

		void Run();

		virtual void UpdateContent(float InDeltaTime) = 0;

		virtual void RenderMainView(Mox::CommandList& InCmdList, const Mox::ContextView& InMainView) = 0;

		uint64_t GetCurrentFrameNumber();

		static constexpr uint32_t GetMaxConcurrentFramesNum() { return Mox::Constants::g_MaxConcurrentFramesNum; };

		virtual void OnQuitApplication();
	protected:

		virtual void OnInitializeContent() = 0;

		// Singleton : Default constructor, copy constructor and assingment operators to be private
		Application();

		static std::unique_ptr<Application> m_Instance; //Note: This is just a declaration, not a definition! m_Instance must be explicitly defined

		bool CanComputeFrame();

		// Scene Related

		Mox::Entity& AddEntity();

		// Render related

		void SetAspectRatio(float InAspectRatio);
		void SetFov(float InFov);

		void OnMainWindowClose();
		void OnWindowPaint();
		void OnWindowResize(uint32_t InNewWidth, uint32_t InNewHeight);

		Mox::Window* m_MainWindow;


		bool m_IsInitialized = false;

		std::unique_ptr<Mox::SimulatonThread> m_Simulator;


	private:
		Application(const Application&) = delete; // We do not want Application to be copiable
		Application& operator=(const Application&) = delete; // We do not want Application to be copy assignable

		bool m_PaintStarted = false;
	};
}
#endif // Application_h__