/*
 MoxMeshComponent.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxMeshComponent.h"
#include "MoxGeometry.h"
#include "Application.h"
#include "MoxEntity.h"

namespace Mox {



	MeshComponent::~MeshComponent()
	{

	}

	MeshComponent::MeshComponent(DrawableCreationInfo&& InCreationInfo)
		: m_VertexBuffer(*InCreationInfo.m_VertexBuffer), m_IndexBuffer(*InCreationInfo.m_IndexBuffer), 
		m_OwnerEntity(*InCreationInfo.m_OwningEntity),
		m_MvpBuffer(std::make_unique<Mox::ConstantBuffer>(Mox::BUFFER_ALLOC_TYPE::DYNAMIC, sizeof(Mox::Matrix4f)))
	{

		InCreationInfo.m_ShaderParameters.emplace_back(Mox::HashSpName("mvp"), m_MvpBuffer.get());

		m_ShaderParameters = InCreationInfo.m_ShaderParameters;
		
		Matrix4f newMvpMatrix = Application::Get()->GetViewProjectionMatrix() * m_OwnerEntity.GetModelMatrix();

		m_MvpBuffer->SetData(newMvpMatrix.data(), sizeof(newMvpMatrix));

		Mox::RequestDrawable(InCreationInfo);
	}

	void MeshComponent::OnPossessedBy(Mox::Entity& InEntity)
	{
		
	}

	void MeshComponent::OnEntityTransformChanged(const Mox::Matrix4f& InNewModelMat)
	{
		// Here eventually we can have a local transform for the mesh that offsets the entity transform.

		Matrix4f newMvpMatrix = Application::Get()->GetViewProjectionMatrix() * m_OwnerEntity.GetModelMatrix();

		m_MvpBuffer->SetData(newMvpMatrix.data(), sizeof(newMvpMatrix));
	}

}
