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
	: m_WorldMatrix(Mox::ModelMatrix(InInfo.WorldPosition, InInfo.WorldScale, InInfo.WorldRotation)),
	m_WorldPos(InInfo.WorldPosition),m_WorldScale(Mox::Matrix3f::Identity()),m_WorldRot(Mox::Matrix3f::Identity())
{
	SetScale(InInfo.WorldScale.x(), InInfo.WorldScale.y(), InInfo.WorldScale.z());
	Rotate(InInfo.WorldRotation.x(), InInfo.WorldRotation.y());

	m_RenderProxy = std::make_shared<Mox::RenderProxy>();

	Mox::RequestRenderProxyForEntity(*this);
}

void Entity::AddComponent(std::shared_ptr<Mox::Component> InComponent)
{
	InComponent->OnPossessedBy(*this);

	m_Components.push_back(InComponent);
}

Entity::~Entity() = default;
Entity::Entity(Entity&&) noexcept = default;

void Entity::Rotate(float InAngleX, float InAngleY)
{

	Mox::Affine3f rotationTransform = Mox::Affine3f::Identity();
	rotationTransform
		.rotate(Mox::AngleAxisf(InAngleY, Mox::Vector3f::UnitX()))
		.rotate(Mox::AngleAxisf(InAngleX, Mox::Vector3f::UnitY()));

	m_WorldRot = rotationTransform.linear() * m_WorldRot;

	// Note: Order of rotations is important (and not commutative). 
	// Multiplication goes from right to left (because transforming points in form of column vectors) and 
	// we always want to start from the previous rotation and adding the new rotation on top of it.
	m_WorldMatrix.topLeftCorner<3, 3>() = m_WorldRot;

	// Re-apply scale
	m_WorldMatrix.topLeftCorner<3, 3>() *= m_WorldScale;

	OnTransformChanged();
}

void Entity::Translate(float InX, float InY, float InZ)
{
	m_WorldPos += Mox::Vector3f(InX, InY, InZ);

	m_WorldMatrix.topRightCorner<3, 1>() = m_WorldPos;

	OnTransformChanged();
}

void Entity::SetScale(float InX, float InY, float InZ)
{
	m_WorldScale.diagonal() = Mox::Vector3f(InX, InY, InZ);

	m_WorldMatrix.topLeftCorner<3, 3>() = m_WorldRot;

	m_WorldMatrix.topLeftCorner<3, 3>() *= m_WorldScale;

	OnTransformChanged();
}

void Entity::OnTransformChanged()
{
	for (std::shared_ptr<class Mox::Component>& curComponent : m_Components)
	{
		curComponent->OnEntityTransformChanged(m_WorldMatrix);
	}
}

}
