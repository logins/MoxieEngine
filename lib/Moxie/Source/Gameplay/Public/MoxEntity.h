/*
 Entity.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Entity_h__
#define Entity_h__

#include "MoxMath.h"

namespace Mox {

class Drawable;
class RenderProxy;
class Component;

// Defines all the possible input parameters for the creation of an entity
struct EntityCreationInfo {
	EntityCreationInfo()
		: WorldPosition(Mox::Vector3f::Zero()), 
		WorldRotation(Mox::Vector3f::Zero()), WorldScale(Mox::Vector3f(1, 1, 1)) {}
	EntityCreationInfo(const Mox::Vector3f& InWoPos)
		: WorldPosition(InWoPos), 
		WorldRotation(Mox::Vector3f::Zero()), WorldScale(Mox::Vector3f(1,1,1)) {}
	EntityCreationInfo(
		const Mox::Vector3f& InWoPos, const Mox::Vector3f& InWoRot, const Mox::Vector3f& InWoScale)
		: WorldPosition(InWoPos), WorldRotation(InWoRot), WorldScale(InWoScale) {}
	Mox::Vector3f WorldPosition;
	Mox::Vector3f WorldRotation;
	Mox::Vector3f WorldScale;
};

// Object existing into a World
class Entity
{
public:
	
	Entity(const Mox::EntityCreationInfo& InInfo);

	// Note: I needed to define both destructor AND move constructor
	// in the implementation file to avoid having a compile error given
	// by forward declaring the template parameter of the unique_ptr 
	// we are using in the member variables!
	virtual ~Entity();
	Entity(Entity&&) noexcept;

	// The entity is will take ownership of the component
	void AddComponent(std::shared_ptr<class Mox::Component> InComponent);

	std::shared_ptr<Mox::RenderProxy> GetRenderProxy() const { return m_RenderProxy; };

	inline Mox::Matrix4f GetModelMatrix() const { return m_WorldMatrix; }

	void Rotate(float InAngleX, float InAngleY);

	void Translate(float InX, float InY, float InZ);

	void SetScale(float InX, float InY, float InZ);

private:

	void OnTransformChanged();

	std::vector<std::shared_ptr<class Mox::Component>> m_Components;

	Mox::Matrix4f m_WorldMatrix;

	Mox::Vector3f m_WorldPos;
	Mox::Matrix3f m_WorldRot;
	Mox::Matrix3f m_WorldScale;

	std::shared_ptr<Mox::RenderProxy> m_RenderProxy;
};

}

#endif // Entity_h__