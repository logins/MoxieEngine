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

struct MeshCreationInfo
{
	Mox::VertexBuffer* m_VertexBuffer;
	Mox::IndexBuffer* m_IndexBuffer;
	std::vector<std::tuple<Mox::SpHash, Mox::Buffer*>> m_ShaderParameters;
};

class Mesh
{
public:
	Mesh(const Mox::VertexBuffer& InVertexBuffer, const Mox::IndexBuffer& InIndexBuffer, 
		const std::vector<std::tuple<Mox::SpHash, Mox::Buffer*>>& InShaderParams);

	void SetShaderParamValue(Mox::SpHash InHash, Mox::Buffer* InBuffer);

	// Translation, Rotation and Scale
	Mox::Matrix4f m_ModelMatrix;

	// Vertex and Index data
	Mox::VertexBufferView& m_VertexBufferView;
	Mox::IndexBufferView& m_IndexBufferView;


	// Shader parameter hash -> Buffer
	std::unordered_map<Mox::SpHash, Mox::Buffer*> m_ShaderParameters; // TODO later replace with Mox::ResourceView*


	Mox::Material& m_Material;
};

}

#endif // MoxMesh_h__
