/*
 ContextView.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef ContextView_h__
#define ContextView_h__

#include "MoxGeometry.h"
#include "GraphicsUtils.h"

namespace Mox {

	// Data about a view on a 3D scene and how to render it within a window
	struct ContextView
	{
		ContextView(){}
		ContextView(float InZMin, float InZMax, float InFov, float InFrameWidth, float InFrameHeight)
			: m_Fov(InFov), m_ZMin(InZMin), m_ZMax(InZMax), m_AspectRatio(InFrameWidth/InFrameHeight)
		{
			const Eigen::Vector3f eyePosition = Eigen::Vector3f(0, 0, -10);
			const Eigen::Vector3f focusPoint = Eigen::Vector3f(0, 0, 0);
			const Eigen::Vector3f upDirection = Eigen::Vector3f(0, 1, 0);
			m_ViewMatrix = Mox::LookAt(eyePosition, focusPoint, upDirection);

			m_ProjMatrix = Mox::Perspective(InZMin, InZMax, m_AspectRatio, InFov);

			m_ScissorRect = Mox::AllocateRect(0l, 0l, LONG_MAX, LONG_MAX); // TODO we probably do not need to allocate platform specific scissor rect and viewport, we can just create the platform specific version when needed

			m_Viewport = Mox::AllocateViewport(0.f, 0.f, static_cast<float>(InFrameWidth), static_cast<float>(InFrameHeight));
		}

		void SetFov(float InFov)
		{
			m_Fov = InFov;
			// Projection Matrix needs updating
			m_ProjMatrix = Mox::Perspective(m_ZMin, m_ZMax, m_AspectRatio, m_Fov);
		}

		void SetAspectRatio(float InAspectRatio)
		{
			m_AspectRatio = InAspectRatio;
			// Projection Matrix needs updating
			m_ProjMatrix = Mox::Perspective(m_ZMin, m_ZMax, m_AspectRatio, m_Fov);
		}

		void SetFrameDimension(float InFrameWidth, float InFrameHeight)
		{
			m_AspectRatio = InFrameWidth / InFrameHeight;
			// Viewport needs updating
			m_Viewport = Mox::AllocateViewport(0.f, 0.f, static_cast<float>(InFrameWidth), static_cast<float>(InFrameHeight));
		}

		float m_Fov, m_ZMin, m_ZMax, m_AspectRatio;

		Mox::Matrix4f m_ProjMatrix;
		Mox::Matrix4f m_ViewMatrix;

		std::unique_ptr<Mox::Rect> m_ScissorRect;
		std::unique_ptr<Mox::ViewPort> m_Viewport;
	};

}

#endif // ContextView_h__