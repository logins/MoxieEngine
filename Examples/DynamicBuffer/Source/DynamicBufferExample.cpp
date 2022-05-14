/*
 DynamicBufferExample.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "DynamicBufferExample.h"
#include <iostream>
#include <algorithm>
#include "GraphicsUtils.h"
#include "PipelineState.h"
#include "MoxGeometry.h"
#include "GraphicsAllocator.h"
#include "CommandQueue.h"
#include "Window.h"
#include "Device.h"
#include "CommandList.h"
#include "MoxMath.h"
#include "Simulator.h"
#include "Renderer.h"
#include "MoxEntity.h"
#include "MoxMesh.h"


#define PART3_SHADERS_PATH(NAME) LQUOTE(DYN_BUF_EXAMPLE_PROJ_ROOT_PATH/shaders/NAME)

int main()
{
	std::cout << "Dynamic Buffer Example" << std::endl;

	Mox::Application::Create<DynBufExampleApp>();

	Mox::Application::Get()->Initialize();

	Mox::Application::Get()->Run();

	Mox::GetDevice().ShutDown();

	// The following will trigger a breakpoint if we have some interfaces to graphics objects that were not cleaned up(leaking)!
	Mox::GetDevice().ReportLiveObjects();

}

DynBufExampleApp::DynBufExampleApp() = default;

void DynBufExampleApp::OnInitializeContent()
{		
	// Load Content
	Mox::CommandList& loadContentCmdList = m_Renderer->GetCmdQueue()->GetAvailableCommandList(); // TODO renderer should not need to be exposed to the derived application

	m_VertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(loadContentCmdList, m_VertexData, sizeof(VertexPosColor), sizeof(m_VertexData)); // TODO can we deduce these last two elements from compiler??
	m_IndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(loadContentCmdList, m_IndexData, sizeof(unsigned short), sizeof(m_IndexData));



	// Executing command list and waiting for full execution
	m_Renderer->GetCmdQueue()->ExecuteCmdList(loadContentCmdList); // TODO renderer should not need to be exposed to the derived application

	m_Renderer->GetCmdQueue()->Flush(); // TODO renderer should not need to be exposed to the derived application

	// Create a mesh and assign it to a new world entity


	std::vector<std::tuple<Mox::SpHash, Mox::Buffer*>> meshShaderParamDefinitions;

	// Creating the buffer for the model matrix
	Mox::Matrix4f modelMatrix = Mox::ModelMatrix(Mox::Vector3i(0, 0, 0), Mox::Vector3f(1, 1, 1), Mox::Vector3f(0, 0, 0));
	Mox::Buffer* mvpBuffer = new Mox::Buffer(Mox::BUFFER_TYPE::DYNAMIC, sizeof(modelMatrix));

	const Eigen::Vector3f eyePosition = Eigen::Vector3f(0, 0, -10);
	const Eigen::Vector3f focusPoint = Eigen::Vector3f(0, 0, 0);
	const Eigen::Vector3f upDirection = Eigen::Vector3f(0, 1, 0);
	Mox::Matrix4f viewMatrix = Mox::LookAt(eyePosition, focusPoint, upDirection);
	// z_min z_max aspect_ratio fov
	Mox::Matrix4f projMatrix = Mox::Perspective(0.1f, 100.f, 1.3333f, 0.7853981634f);

	Mox::Matrix4f mvpValue = projMatrix * viewMatrix * modelMatrix;

	mvpBuffer->SetData(mvpValue.data(), sizeof(mvpValue));

	// Setting the relative shader parameter
	meshShaderParamDefinitions.emplace_back(Mox::HashSpName("mvp"), mvpBuffer);

	// Creating buffer for the color mod
	m_ColorModBuffer = std::make_unique<Mox::Buffer>(Mox::BUFFER_TYPE::DYNAMIC, sizeof(float));
	float colorMod = .5f;
	m_ColorModBuffer->SetData(&colorMod, sizeof(float));
	// Setting the relative shader parameter
	meshShaderParamDefinitions.emplace_back(Mox::HashSpName("c_mod"), m_ColorModBuffer.get());

	std::vector<Mox::MeshCreationInfo> meshCreationInfo{ {m_VertexBuffer, m_IndexBuffer, meshShaderParamDefinitions} };

	m_CubeEntity = &AddEntity({ Mox::Vector3i(0,0,0), meshCreationInfo });

	// Window events delegates
	m_MainWindow->OnMouseMoveDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnMouseMove>(this);
	m_MainWindow->OnMouseWheelDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnMouseWheel>(this);
	m_MainWindow->OnTypingKeyDownDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnTypingKeyPressed>(this);
	m_MainWindow->OnControlKeyDownDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnControlKeyPressed>(this);

}

void DynBufExampleApp::OnQuitApplication()
{
	Mox::Application::OnQuitApplication();

}

void DynBufExampleApp::OnMouseWheel(float InDeltaRot)
{
	// TODO this needs to be passed by message to the renderer
	//SetFov(std::max(0.2094395102f, std::min(m_MainWindowView.m_Fov -= InDeltaRot / 1200.f, 1.570796327f))); // clamping
}

void DynBufExampleApp::OnMouseMove(int32_t InX, int32_t InY)
{
	static int32_t prevX = 0, prevY = 0;
	if (m_MainWindow->IsMouseRightHold())
		OnRightMouseDrag(InX - prevX, InY - prevY);
	if (m_MainWindow->IsMouseLeftHold())
		OnLeftMouseDrag(InX - prevX, InY - prevY);

	prevX = InX; prevY = InY;
}

void DynBufExampleApp::OnLeftMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	Mox::AffineTransform<float, 3> tr;  // TODO use part of the model matrix for rotation instead of creating a new one!
	tr.setIdentity();
	tr.rotate(Mox::AngleAxisf(-InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), Mox::Vector3f::UnitY()))
		.rotate(Mox::AngleAxisf(-InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), Mox::Vector3f::UnitX()));

	m_CubeEntity->MultiplyModelMatrix(tr.matrix());
}

void DynBufExampleApp::OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	Eigen::Transform<float, 3, Eigen::Affine> tr;  // TODO use part of the model matrix for translation instead of creating a new one!
	tr.setIdentity();
	tr.translate(Eigen::Vector3f(InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), -InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), 0));

	m_CubeEntity->MultiplyModelMatrix(tr.matrix());
}

void DynBufExampleApp::OnTypingKeyPressed(Mox::KEYBOARD_KEY InKeyPressed)
{
	if (InKeyPressed == Mox::KEYBOARD_KEY::KEY_V)
		m_MainWindow->SetVSyncEnabled(!m_MainWindow->IsVSyncEnabled());
}

void DynBufExampleApp::OnControlKeyPressed(Mox::KEYBOARD_KEY InPressedSysKey)
{
	if (InPressedSysKey == Mox::KEYBOARD_KEY::KEY_ESC)
		m_MainWindow->Close();
}

void DynBufExampleApp::UpdateContent(float InDeltaTime)
{
	// Updating color modifier
	static float progress, counter = 0.f;
	counter = 0.75f + std::sin(progress) / 2.f;
	progress += 0.1f * InDeltaTime;

	m_ColorModBuffer->SetData(&counter, sizeof(float)); // TODO template SetData

}


