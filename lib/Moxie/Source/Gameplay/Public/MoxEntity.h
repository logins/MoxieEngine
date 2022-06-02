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
	Mox::Vector3i WorldPosition;

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
	void AddComponent(std::unique_ptr<class Mox::Component> InComponent);

	Mox::RenderProxy* GetRenderProxy() const { return m_RenderProxy; };

	void MultiplyModelMatrix(const Mox::Matrix4f& InNewModelMatrix);

	void SetRenderProxy(Mox::RenderProxy* InProxy) { m_RenderProxy = InProxy; }

private:
	void OnRotation(const Mox::Vector3f& InAxis, float InAngle);
	void OnTraslation(const Mox::Vector3f& InTraslation);
	void OnScale(const Mox::Vector3f& InScale);

	std::vector<std::unique_ptr<class Mox::Component>> m_Components;


	Mox::Vector3i m_WorldPos;

	Mox::RenderProxy* m_RenderProxy;
};

}

#endif // Entity_h__