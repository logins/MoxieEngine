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
/*
	m_CubemapVertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(m_CubemapVertexLayoutDesc, m_CubemapVertexData, sizeof(CubemapVertex), sizeof(m_CubemapVertexData)); // TODO can we deduce these last two elements from compiler??
	m_CubemapIndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(m_CubemapIndexData, sizeof(unsigned short), sizeof(m_CubemapIndexData));

	m_CubeEntity = &AddEntity({ Mox::Vector3f::Zero() });

	// Create the cubemap texture
	m_CubemapTexture = std::make_unique<Mox::Texture>(TEXTURES_EXAMPLE_CONTENT_PATH(CubeMap.dds));

	// Set it as shader parameter
	Mox::TextureMeshParams meshShaderParamDefinitions {
		{Mox::HashSpName("cube_tex"), m_CubemapTexture.get()}
	};

	std::unique_ptr<Mox::MeshComponent> cubeMesh = std::make_unique<Mox::MeshComponent>(
		Mox::DrawableCreationInfo{ 
			m_CubeEntity, m_CubemapVertexBuffer, m_CubemapIndexBuffer, Mox::BufferMeshParams(), std::move(meshShaderParamDefinitions) });
	
	m_CubeEntity->AddComponent(std::move(cubeMesh));
	*/

	// ----- QUAD -----

	m_QuadVertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(
		m_QuadVertexLayoutDesc, m_QuadVertexData, sizeof(QuadVertex), sizeof(m_QuadVertexData)); // TODO can we deduce these last two elements from compiler??
	m_QuadIndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(
		m_QuadIndexData, sizeof(unsigned short), sizeof(m_QuadIndexData));

	m_QuadEntity = &AddEntity({ Mox::Vector3f::Zero() });

	// Create the Quad texture
	// Texture courtesy of https://www.solarsystemscope.com/textures/
	m_QuadTexture = std::make_unique<Mox::Texture>(TEXTURES_EXAMPLE_CONTENT_PATH(MarsMap.dds));
	// Set it as shader parameter
	Mox::TextureMeshParams meshShaderParamDefinitions{
		{Mox::HashSpName("albedo_tex"), m_QuadTexture.get()}
	};
	// Create mesh component and add it to the entity
	std::unique_ptr<Mox::MeshComponent> quadMesh = std::make_unique<Mox::MeshComponent>(
		Mox::DrawableCreationInfo{
			m_QuadEntity, m_QuadVertexBuffer, m_QuadIndexBuffer, Mox::BufferMeshParams(), std::move(meshShaderParamDefinitions) });

	m_QuadEntity->AddComponent(std::move(quadMesh));


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
	m_QuadEntity->SetScale(curScale, 1.0, 1.0);


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
	m_QuadEntity->Rotate(
		-InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()),
		-InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()));
}

void TexturesExampleApplication::OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	m_QuadEntity->Translate(InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), -InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), 0.f);
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