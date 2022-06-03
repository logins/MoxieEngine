/*
 MoxEntity.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxEntity.h"
#include "MoxRenderProxy.h"
#include "MoxDrawable.h"
#include "MoxComponent.h"
#include "MoxGeometry.h"

namespace Mox {


Entity::Entity(const Mox::EntityCreationInfo& InInfo)
	: m_WorldMatrix(Mox::ModelMatrix(InInfo.WorldPosition, Mox::Vector3f(1, 1, 1), Mox::Vector3f(0, 0, 0)))
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

void Entity::Rotate(float InAngleX, float InAngleY)
{
	m_WorldRot[0] = std::fmod(m_WorldRot[0] + InAngleX, 360.f);
	m_WorldRot[1] = std::fmod(m_WorldRot[1] + InAngleX, 360.f);

	Mox::Affine3f rotationTransform = Mox::Affine3f::Identity();
	rotationTransform
		.rotate(Mox::AngleAxisf(InAngleY, Mox::Vector3f::UnitX()))
		.rotate(Mox::AngleAxisf(InAngleX, Mox::Vector3f::UnitY()));

	m_WorldMatrix.topLeftCorner<3, 3>() *= rotationTransform.linear();

	for (std::unique_ptr<class Mox::Component>& curComponent : m_Components)
	{
		curComponent->OnEntityTransformChanged(m_WorldMatrix);
	}

}

//Mox::Matrix4f& Entity::GetModelMatrix()
//{
//	return 
//}

}
