/*
 MoxieLogoScene.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxieLogoScene.h"
#include <iostream>
#include "Device.h"
#include "GraphicsAllocator.h"
#include "MoxEntity.h"
#include "MoxMeshComponent.h"


#define MOXIE_LOGO_CONTENT_PATH(NAME) LQUOTE(MOXIE_LOGO_PROJ_ROOT_PATH/Content/NAME)

int main()
{
	std::cout << "Moxie Example Application: Moxie Logo!" << std::endl;

	Mox::Application::Create<MoxieLogoSceneApp>();

	Mox::Application::Get()->Initialize();

	Mox::Application::Get()->Run();

	Mox::GetDevice().ShutDown();

	// The following will trigger a breakpoint if we have some interfaces to graphics objects that were not cleaned up(leaking)!
	Mox::GetDevice().ReportLiveObjects();
}




void MoxieLogoSceneApp::OnInitializeContent()
{
	// Load Content

	// ----- SKYDOME -----

	static std::vector<Mox::Vector3f> skydomeMeshVertices;
	static std::vector<Mox::Vector2f> skydomeMeshUvs;
	static std::vector<uint16_t> skydomeMeshIndices;
	Mox::UVSphere(15, 15, skydomeMeshVertices, skydomeMeshUvs, skydomeMeshIndices);
	//Mox::NormalizedCube(15, skydomeMeshVertices, skydomeMeshUvs, skydomeMeshIndices);
	static std::vector<TexVertexType> skydomeVbData;
	skydomeVbData.reserve(skydomeMeshVertices.size());
	auto uvsIt = skydomeMeshUvs.begin();
	for (const Mox::Vector3f& pos : skydomeMeshVertices)
	{
		skydomeVbData.push_back({ pos, pos }); // Using vertex local position as cube UV coordinates
		++uvsIt;
	}

	m_SkydomeVertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(
		m_VertexLayoutDesc,
		skydomeVbData.data(),
		sizeof(TexVertexType),
		sizeof(TexVertexType) * skydomeVbData.size()
	); // TODO can we deduce these last two elements from compiler??

	m_SkydomeIndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(
		skydomeMeshIndices.data(),
		sizeof(uint16_t),
		sizeof(uint16_t) * skydomeMeshIndices.size()
	);

	m_SkydomeEntity = &AddEntity(Mox::EntityCreationInfo{ 
		Mox::Vector3f::Zero(), Mox::Vector3f::Zero(),Mox::Vector3f(10,10,10)});

	// Create the skydome cube texture
	// Texture courtesy of https://www.solarsystemscope.com/textures/
	m_SkydomeCubeTexture = std::make_unique<Mox::Texture>(MOXIE_LOGO_CONTENT_PATH(CubeMilkyWay.dds));
	// Set it as shader parameter
	Mox::TextureMeshParams meshShaderParamDefinitions{
		{Mox::HashSpName("albedo_cube"), m_SkydomeCubeTexture.get()}
	};
	// Create mesh component and add it to the entity
	static std::unique_ptr<Mox::MeshComponent> skydomeMesh = std::make_unique<Mox::MeshComponent>(
		Mox::DrawableCreationInfo{
			m_SkydomeEntity, m_SkydomeVertexBuffer, m_SkydomeIndexBuffer,
			Mox::BufferMeshParams(), std::move(meshShaderParamDefinitions), 
			true // Render backfaces
		});

	m_SkydomeEntity->AddComponent(std::move(skydomeMesh));
	// ----- ENDS SKYDOME -----

	//// ----- QUAD -----

	//m_QuadVertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(
	//	m_VertexLayoutDesc,
	//	&m_QuadVertexData,
	//	sizeof(TexVertexType),
	//	sizeof(m_QuadVertexData)
	//);

	//m_QuadIndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(
	//	&m_QuadIndexData,
	//	sizeof(uint16_t),
	//	sizeof(m_QuadIndexData)
	//);

	//m_QuadEntity = &AddEntity(Mox::EntityCreationInfo{ Mox::Vector3f(2.5f,-2.5f,0),Mox::Vector3f::Zero(), Mox::Vector3f(1.5f,1.5f,1.5f) });

	//m_QuadTexture = std::make_unique<Mox::Texture>(MOXIE_LOGO_CONTENT_PATH(MarsMap.dds));
	//// Set it as shader parameter
	//Mox::TextureMeshParams quadMeshShaderParamDefinitions = Mox::TextureMeshParams{
	//	{Mox::HashSpName("albedo_tex"), m_QuadTexture.get()}
	//};
	//// Create mesh component and add it to the entity
	//std::unique_ptr<Mox::MeshComponent> quadMesh = std::make_unique<Mox::MeshComponent>(
	//	Mox::DrawableCreationInfo{
	//		m_QuadEntity, m_QuadVertexBuffer, m_QuadIndexBuffer,
	//		Mox::BufferMeshParams(), std::move(quadMeshShaderParamDefinitions),
	//	});

	//m_QuadEntity->AddComponent(std::move(quadMesh));

	//// ----- QUAD ENDS -----

	//// ----- SPHERE -----

	//static std::vector<Mox::Vector3f> sphereMeshVertices;
	//static std::vector<Mox::Vector2f> sphereMeshUvs;
	//static std::vector<uint16_t> sphereMeshIndices;
	//Mox::UVSphere(15, 15, sphereMeshVertices, sphereMeshUvs, sphereMeshIndices);
	////Mox::NormalizedCube(15, sphereMeshVertices, sphereMeshUvs, sphereMeshIndices);
	//static std::vector<TexVertexType> sphereVbData;
	//sphereVbData.reserve(sphereMeshVertices.size());
	//uvsIt = sphereMeshUvs.begin();
	//for (const Mox::Vector3f& pos : sphereMeshVertices)
	//{
	//	sphereVbData.push_back({ pos, pos }); // Using vertex local position as cube UV coordinates
	//	++uvsIt;
	//}

	//m_SphereVertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(
	//	m_VertexLayoutDesc,
	//	sphereVbData.data(),
	//	sizeof(TexVertexType),
	//	sizeof(TexVertexType) * sphereVbData.size()
	//); // TODO can we deduce these last two elements from compiler??

	//m_SphereIndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(
	//	sphereMeshIndices.data(),
	//	sizeof(uint16_t),
	//	sizeof(uint16_t) * sphereMeshIndices.size()
	//);

	//m_SphereEntity = &AddEntity({ Mox::Vector3f::Zero() });

	//// Create the sphere cube texture
	//// Texture courtesy of https://www.solarsystemscope.com/textures/
	//m_SphereCubeTexture = std::make_unique<Mox::Texture>(MOXIE_LOGO_CONTENT_PATH(CubeEarthNight.dds));
	//// Set it as shader parameter
	//Mox::TextureMeshParams sphereMeshShaderParamDefinitions{
	//	{Mox::HashSpName("albedo_cube"), m_SphereCubeTexture.get()}
	//};
	//// Create mesh component and add it to the entity
	//static std::unique_ptr<Mox::MeshComponent> sphereMesh = std::make_unique<Mox::MeshComponent>(
	//	Mox::DrawableCreationInfo{
	//		m_SphereEntity, m_SphereVertexBuffer, m_SphereIndexBuffer,
	//		Mox::BufferMeshParams(), std::move(sphereMeshShaderParamDefinitions),
	//	});

	//m_SphereEntity->AddComponent(std::move(sphereMesh));
	//// ----- ENDS SPHERE -----



}

void MoxieLogoSceneApp::UpdateContent(float InDeltaTime)
{
	float rotationSpeed = 0.001f;

	m_SkydomeEntity->Rotate(InDeltaTime * rotationSpeed, 0.f);
}

