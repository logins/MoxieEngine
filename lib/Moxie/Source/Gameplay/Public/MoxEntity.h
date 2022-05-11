/*
 Entity.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef Entity_h__
#define Entity_h__

#include "MoxMath.h"
#include "MoxMesh.h" // TODO find a way to remove this dependency


namespace Mox {

class Mesh;
class RenderProxy;

// Defines all the possible input parameters for the creation of an entity
struct EntityCreationInfo {
	Mox::Vector3i WorldPosition;

	std::vector<struct Mox::MeshCreationInfo> m_MeshCreationInfo;
};

// Object existing into a World
class Entity
{
public:

	
	Entity(const Mox::EntityCreationInfo& InInfo);

	Mox::RenderProxy* GetRenderProxy() const { return m_RenderProxy; };

	void MultiplyModelMatrix(const Mox::Matrix4f& InNewModelMatrix);

	void SetRenderProxy(Mox::RenderProxy* InProxy) { m_RenderProxy = InProxy; }

private:
	Mox::Vector3i m_WorldPos;

	Mox::RenderProxy* m_RenderProxy;
};

}

#endif // Entity_h__