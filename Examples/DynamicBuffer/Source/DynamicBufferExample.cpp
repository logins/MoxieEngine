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
#include "MoxDrawable.h"
#include "MoxMeshComponent.h"


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

	m_VertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(m_VertexData, sizeof(VertexPosColor), sizeof(m_VertexData)); // TODO can we deduce these last two elements from compiler??
	m_IndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(m_IndexData, sizeof(unsigned short), sizeof(m_IndexData));



	// Executing command list and waiting for full execution

	// Create a mesh and assign it to a new world entity


	std::vector<std::tuple<Mox::SpHash, Mox::ConstantBuffer*>> meshShaderParamDefinitions;

	// Creating the buffer for the model matrix
	






	// Creating buffer for the color mod
	m_ColorModBuffer = std::make_unique<Mox::ConstantBuffer>(Mox::BUFFER_ALLOC_TYPE::DYNAMIC, sizeof(float));
	float colorMod = .5f;
	m_ColorModBuffer->SetData(&colorMod, sizeof(float));
	// Setting the relative shader parameter
	meshShaderParamDefinitions.emplace_back(Mox::HashSpName("c_mod"), m_ColorModBuffer.get());

	m_CubeEntity = &AddEntity({ Mox::Vector3f::Zero() });

	std::unique_ptr<Mox::MeshComponent> cubeMesh = std::make_unique<Mox::MeshComponent>(
		Mox::DrawableCreationInfo{ m_CubeEntity, m_VertexBuffer, m_IndexBuffer, std::move(meshShaderParamDefinitions)} );


	m_CubeEntity->AddComponent(std::move(cubeMesh));

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
	static float curScale = 1.0f;
	curScale = std::clamp<float>(curScale + (InDeltaRot > 0 ? 0.4f : -0.4f), 0.1f, 3.f);
	m_CubeEntity->SetScale(curScale, 1.0, 1.0);


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
	m_CubeEntity->Rotate(
		-InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()),
		-InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()));
}

void DynBufExampleApp::OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	m_CubeEntity->Translate(InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), -InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), 0.f);
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


