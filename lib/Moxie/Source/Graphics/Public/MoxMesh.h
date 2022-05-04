/*
 MoxMesh.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#ifndef MoxMesh_h__
#define MoxMesh_h__

#include "MoxMath.h"
#include <unordered_map>

namespace Mox {

	class VertexBuffer;
	class IndexBuffer;
	class Material;
	struct ConstantBufferView;
	struct VertexBufferView;
	struct IndexBufferView;

class Mesh
{
public:
	Mesh(const Mox::Vector3i& InPosition, const Mox::Vector3f& InRotation, Mox::VertexBufferView& InVertexBufferView, Mox::IndexBufferView& InIndexBufferView);

	void SetShaderParamValue(Mox::SpHash InHash, Mox::ConstantBufferView* InCbv);

	// Translation, Rotation and Scale
	Mox::Matrix4f m_ModelMatrix;

	// Vertex and Index data
	Mox::VertexBufferView& m_VertexBufferView;
	Mox::IndexBufferView& m_IndexBufferView;


	// Shader parameter hash -> Resource View
	std::unordered_map<Mox::SpHash, Mox::ConstantBufferView*> m_ShaderParameters; // TODO later replace with Mox::ResourceView*


	Mox::Material& m_Material;
};

}

#endif // MoxMesh_h__
