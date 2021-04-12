/*
 Application.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Application_h__
#define Application_h__

#include "MoxUtils.h"
#include "MoxMath.h"

namespace Mox {
	
		class Device;
		class CommandQueue;
		class CommandList;
		class Window;
		class GraphicsAllocatorBase;
		struct Rect;
		struct ViewPort;

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

		virtual void Initialize();

		void Run();

		static uint64_t GetCurrentFrameNumber() { return m_CpuFrameNumber; };

		static constexpr uint32_t GetMaxConcurrentFramesNum() { return Mox::Constants::g_MaxConcurrentFramesNum; };

		virtual void OnQuitApplication();
	protected:
		virtual void OnCpuFrameStarted();

		virtual void OnCpuFrameFinished();

		// Singleton : Default constructor, copy constructor and assingment operators to be private
		Application();

		static std::unique_ptr<Application> m_Instance; //Note: This is just a declaration, not a definition! m_Instance must be explicitly defined

		virtual void UpdateContent(float InDeltaTime) = 0;

		virtual void RenderContent(Mox::CommandList& InCmdList) = 0;

		void SetAspectRatio(float InAspectRatio);
		void SetFov(float InFov);

		void OnMainWindowClose();
		void OnWindowPaint();
		void OnWindowResize(uint32_t InNewWidth, uint32_t InNewHeight);

		Mox::Device& m_GraphicsDevice;

		std::unique_ptr<Mox::GraphicsAllocatorBase> m_GraphicsAllocator;

		Mox::CommandQueue* m_CmdQueue;

		Mox::Window* m_MainWindow;

		std::unique_ptr<Mox::Rect> m_ScissorRect = nullptr;

		std::unique_ptr<Mox::ViewPort> m_Viewport = nullptr;

		bool m_IsInitialized = false;


		float m_Fov = 0.f;
		float m_ZMin = 0.f, m_ZMax = 0.f;
		float m_AspectRatio = 0;

		// Model, View, Projection Matrices
		// Note: View and Projection matrices would belong to a camera class if there was one
		// Model matrix would belong to a specific entity in the scene
		Mox::Matrix4f m_ModelMatrix;
		Mox::Matrix4f m_ViewMatrix;
		Mox::Matrix4f m_ProjMatrix;

		bool m_PaintStarted = false;

		static uint64_t m_CpuFrameNumber;

	private:
		Application(const Application&) = delete; // We do not want Application to be copiable
		Application& operator=(const Application&) = delete; // We do not want Application to be copy assignable

		void Update();

		void Render();

		float m_DeltaTime = 0.f;

		bool CanComputeFrame();
	};
}
#endif // Application_h__