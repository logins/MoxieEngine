/*
 MoxEntity.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxEntity.h"
#include "MoxRenderProxy.h"
#include "MoxMesh.h"

namespace Mox {


Entity::Entity(const Mox::EntityCreationInfo& InInfo)
{
	Mox::RequestRenderProxyForEntity(*this, InInfo.m_MeshCreationInfo);
}


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
