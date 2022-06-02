/*
 MoxMeshComponent.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxMeshComponent.h"

namespace Mox {



	MeshComponent::~MeshComponent()
	{

	}

	MeshComponent::MeshComponent(DrawableCreationInfo&& InCreationInfo)
		: m_VertexBuffer(*InCreationInfo.m_VertexBuffer), m_IndexBuffer(*InCreationInfo.m_IndexBuffer), 
		m_ShaderParameters(InCreationInfo.m_ShaderParameters), m_OwnerEntity(*InCreationInfo.m_OwningEntity)
	{
		Mox::RequestDrawable(InCreationInfo);
	}

	void MeshComponent::OnPossessedBy(class Mox::Entity& InEntity)
	{
		
	}

}
