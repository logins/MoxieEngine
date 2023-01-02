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
#include <mutex>

namespace Mox {
	
		class Device;
		class Window;
		class GraphicsAllocatorBase;
		struct Rect;
		struct ViewPort;
		class CommandList;
		class SimulatonThread;
		class RenderThread;
		class Entity;
		struct EntityCreationInfo;

	/*
	 * Represents the whole application run from the executable.
	 * Application acts as a main hub to generate most of the other objects and runs the main engine loop,
	 * while synchronizing simulation and render threads.
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

		// At the moment we don't have a camera system 
		// so the application can have generic view and projection matrices.
		inline Matrix4f GetViewMatrix() const { return m_ViewMatrix; }
		inline float GetFovYRad() const { return m_FovYRad; }
		inline Matrix4f GetProjectionMatrix() const { return m_ProjMatrix; }
		// Precomputed product of view and projection
		inline Matrix4f GetViewProjectionMatrix() const { return m_ViewProjMatrix; }


		uint64_t GetCurrentFrameNumber();

		static constexpr uint32_t GetMaxGpuConcurrentFramesNum() { return Mox::Constants::g_MaxConcurrentFramesNum; };

		virtual void OnQuitApplication();

		bool SyncForFrameStart_SimThread();
		bool SyncForFrameStart_RenderThread();
		void SyncForFrameEnd_SimThread();
		void SyncForFrameEnd_RenderThread();


	protected:

		virtual void OnInitializeContent() = 0;

		// Singleton : Default constructor, copy constructor and assingment operators to be private
		Application();

		//Note: This is just a declaration, not a definition! m_Instance must be explicitly defined
		static std::unique_ptr<Application> m_Instance;

		bool CanComputeFrame();

		// Scene Related

		Mox::Entity& AddEntity(const Mox::EntityCreationInfo& InInfo);

		// Render related

		void SetAspectRatio(float InAspectRatio);

		void OnMainWindowClose();
		void OnWindowPaint();
		void OnWindowResize(uint32_t InNewWidth, uint32_t InNewHeight);

		Mox::Window* m_MainWindow;

		bool m_IsInitialized = false;

		// Inter-thread communication

		std::unique_ptr<Mox::SimulatonThread> m_Simulator;
		std::unique_ptr<Mox::RenderThread> m_Renderer;
		// Used to sync frames numbers between sim and render threads
		std::mutex m_FramesMutex;
		std::condition_variable m_SimToRenderFrameCondVar;
		// The following two variables are used for inter-thread frame syncing between Simulation and Render thread:
		// Simulation frame n will start only when the previous n-1 render frame is done
		// and, at the same time, Render frame will start only when current simulation data has been computed
		uint64_t m_DoneRenderFrameNum;
		uint64_t m_DoneSimFrameNum;

		// Note: m_StagedRenderUpdates is supposed to be accessed in a scope that is locked with the m_FramesMutex
		Mox::FrameRenderUpdates m_StagedRenderUpdates;

		Mox::Matrix4f m_ViewMatrix;
		float m_FovYRad;
		Mox::Matrix4f m_ProjMatrix;
		Mox::Matrix4f m_ViewProjMatrix;

	private:
		Application(const Application&) = delete; // We do not want Application to be copiable
		Application& operator=(const Application&) = delete; // We do not want Application to be copy assignable

		// Will set m_IsTerminating to true so the threads will stop generating frames
		void OrderThreadsTermination();

		bool m_PaintStarted = false;

		float m_SimulationFrameTime;
		float m_RenderFrameTime;

		bool m_IsTerminating = false;

	};
}
#endif // Application_h__