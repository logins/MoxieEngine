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

	static std::vector<Mox::Vector3f> sphereMeshVertices;
	static std::vector<Mox::Vector2f> sphereMeshUvs;
	static std::vector<uint16_t> sphereMeshIndices;
	Mox::UVSphere(15, 15, sphereMeshVertices, sphereMeshUvs, sphereMeshIndices);
	//Mox::NormalizedCube(15, sphereMeshVertices, sphereMeshUvs, sphereMeshIndices);
	static std::vector<QuadVertex> sphereVbData; 
	sphereVbData.reserve(sphereMeshVertices.size());
	auto uvsIt = sphereMeshUvs.begin();
	for (const Mox::Vector3f& pos : sphereMeshVertices)
	{
		sphereVbData.push_back({ pos, pos }); // Using vertex local position as cube UV coordinates
		++uvsIt;
	}

	// ----- QUAD -----

	m_SphereVertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(
		m_SphereVertexLayoutDesc, 
		sphereVbData.data(),
		sizeof(QuadVertex), 
		sizeof(QuadVertex) * sphereVbData.size()
	); // TODO can we deduce these last two elements from compiler??
	
	m_SphereIndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(
		sphereMeshIndices.data(),
		sizeof(uint16_t),
		sizeof(uint16_t) * sphereMeshIndices.size()
	);

	m_QuadEntity = &AddEntity({ Mox::Vector3f::Zero() });

	// Create the Quad texture
	// Texture courtesy of https://www.solarsystemscope.com/textures/
	m_SphereCubeTexture = std::make_unique<Mox::Texture>(TEXTURES_EXAMPLE_CONTENT_PATH(CubeMarsMap.dds));
	// Set it as shader parameter
	Mox::TextureMeshParams meshShaderParamDefinitions{
		{Mox::HashSpName("albedo_cube"), m_SphereCubeTexture.get()}
	};
	// Create mesh component and add it to the entity
	std::unique_ptr<Mox::MeshComponent> quadMesh = std::make_unique<Mox::MeshComponent>(
		Mox::DrawableCreationInfo{
			m_QuadEntity, m_SphereVertexBuffer, m_SphereIndexBuffer, Mox::BufferMeshParams(), std::move(meshShaderParamDefinitions) });

	m_QuadEntity->AddComponent(std::move(quadMesh));
	// ----- ENDS QUAD -----



	// Window events delegates
	m_MainWindow->OnMouseMoveDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnMouseMove>(this);
	m_MainWindow->OnMouseWheelDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnMouseWheel>(this);
	m_MainWindow->OnTypingKeyDownDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnTypingKeyPressed>(this);
	m_MainWindow->OnControlKeyDownDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnControlKeyPressed>(this);
}

void TexturesExampleApplication::OnMouseWheel(float InDeltaRot)
{
	m_QuadEntity->Translate(0, 0, (InDeltaRot > 0 ? 1 : -1));

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