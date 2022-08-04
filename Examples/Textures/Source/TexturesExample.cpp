/*
 TexturesExample.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "TexturesExample.h"
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
#include "MoxEntity.h"
#include "Simulator.h"
#include "Renderer.h"
#include "MoxMeshComponent.h"


//#define TEXTURES_EXAMPLE_SHADERS_PATH(NAME) LQUOTE(TEXTURES_EXAMPLE_PROJ_ROOT_PATH/shaders/NAME)
#define TEXTURES_EXAMPLE_CONTENT_PATH(NAME) LQUOTE(TEXTURES_EXAMPLE_PROJ_ROOT_PATH/Content/NAME)


int main()
{
	std::cout << "Moxie Example Application: Texture Usage!" << std::endl;

	Mox::Application::Create<TexturesExampleApplication>();

	Mox::Application::Get()->Initialize();

	Mox::Application::Get()->Run();

	Mox::GetDevice().ShutDown();

	// The following will trigger a breakpoint if we have some interfaces to graphics objects that were not cleaned up(leaking)!
	Mox::GetDevice().ReportLiveObjects();
}


void TexturesExampleApplication::OnInitializeContent()
{
	// Load Content

	m_VertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(m_VertexLayoutDesc, m_VertexData, sizeof(VertexPosColor), sizeof(m_VertexData)); // TODO can we deduce these last two elements from compiler??
	m_IndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(m_IndexData, sizeof(unsigned short), sizeof(m_IndexData));

	m_CubeEntity = &AddEntity({ Mox::Vector3f::Zero() });

	// Create the cubemap texture
	m_Cubemap = std::make_unique<Mox::Texture>(TEXTURES_EXAMPLE_CONTENT_PATH(CubeMap.dds));

	// Set it as shader parameter
	Mox::TextureMeshParams meshShaderParamDefinitions {
		{Mox::HashSpName("cube_tex"), m_Cubemap.get()}
	};

	std::unique_ptr<Mox::MeshComponent> cubeMesh = std::make_unique<Mox::MeshComponent>(
		Mox::DrawableCreationInfo{ 
			m_CubeEntity, m_VertexBuffer, m_IndexBuffer, Mox::BufferMeshParams(), std::move(meshShaderParamDefinitions) });
	
	m_CubeEntity->AddComponent(std::move(cubeMesh));


	// Window events delegates
	m_MainWindow->OnMouseMoveDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnMouseMove>(this);
	m_MainWindow->OnMouseWheelDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnMouseWheel>(this);
	m_MainWindow->OnTypingKeyDownDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnTypingKeyPressed>(this);
	m_MainWindow->OnControlKeyDownDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnControlKeyPressed>(this);
}

void TexturesExampleApplication::OnMouseWheel(float InDeltaRot)
{
	static float curScale = 1.0f;
	curScale = std::clamp<float>(curScale + (InDeltaRot > 0 ? 0.4f : -0.4f), 0.1f, 3.f);
	m_CubeEntity->SetScale(curScale, 1.0, 1.0);


}

void TexturesExampleApplication::OnMouseMove(int32_t InX, int32_t InY)
{
	static int32_t prevX = 0, prevY = 0;
	if (m_MainWindow->IsMouseRightHold())
		OnRightMouseDrag(InX - prevX, InY - prevY);
	if (m_MainWindow->IsMouseLeftHold())
		OnLeftMouseDrag(InX - prevX, InY - prevY);

	prevX = InX; prevY = InY;
}

void TexturesExampleApplication::OnLeftMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	m_CubeEntity->Rotate(
		-InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()),
		-InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()));
}

void TexturesExampleApplication::OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	m_CubeEntity->Translate(InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), -InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), 0.f);
}

void TexturesExampleApplication::OnTypingKeyPressed(Mox::KEYBOARD_KEY InKeyPressed)
{
	if (InKeyPressed == Mox::KEYBOARD_KEY::KEY_V)
		m_MainWindow->SetVSyncEnabled(!m_MainWindow->IsVSyncEnabled());
}

void TexturesExampleApplication::OnControlKeyPressed(Mox::KEYBOARD_KEY InPressedSysKey)
{
	if (InPressedSysKey == Mox::KEYBOARD_KEY::KEY_ESC)
		m_MainWindow->Close();
}

void TexturesExampleApplication::UpdateContent(float InDeltaTime)
{

	// Updating MVP matrix


}