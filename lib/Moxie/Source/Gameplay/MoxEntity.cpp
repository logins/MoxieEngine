/*
 MoxEntity.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxEntity.h"
#include "MoxRenderProxy.h"
#include "MoxDrawable.h"
#include "MoxComponent.h"

namespace Mox {


Entity::Entity(const Mox::EntityCreationInfo& InInfo)
{
	Mox::RequestRenderProxyForEntity(*this);
}

void Entity::AddComponent(std::unique_ptr<class Mox::Component> InComponent)
{
	InComponent->OnPossessedBy(*this);

	m_Components.emplace_back(std::move(InComponent));
}

Entity::~Entity() = default;
Entity::Entity(Entity&&) noexcept = default;

void Entity::MultiplyModelMatrix(const Mox::Matrix4f& InNewModelMatrix)
{
	Mox::Matrix4f& curModelMat = m_RenderProxy->m_Meshes[0]->m_ModelMatrix; // TODO the entity needs to have its transform and update the relative meshes transform accordingly

	curModelMat = InNewModelMatrix * curModelMat;
}

//Mox::Matrix4f& Entity::GetModelMatrix()
//{
//	return 
//}

}
