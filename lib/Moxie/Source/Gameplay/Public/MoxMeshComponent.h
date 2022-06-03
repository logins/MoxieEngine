/*
 MoxMeshComponent.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxMeshComponent_h__
#define MoxMeshComponent_h__

#include "MoxComponent.h"


namespace Mox {

struct DrawableCreationInfo;

class MeshComponent : public Component
{
public:
	MeshComponent(DrawableCreationInfo&& InCreationInfo);

	virtual ~MeshComponent();

	void OnPossessedBy(class Mox::Entity& InEntity) override;


	void OnEntityTransformChanged(const Mox::Matrix4f& InNewModelMat) override;

private:
	Mox::VertexBuffer& m_VertexBuffer;
	Mox::IndexBuffer& m_IndexBuffer;

	MeshParamsList m_ShaderParameters;

	std::unique_ptr<Mox::ConstantBuffer> m_MvpBuffer;


	Mox::Entity& m_OwnerEntity;

	MeshComponent() = delete;
};

}

#endif // MoxMeshComponent_h__
