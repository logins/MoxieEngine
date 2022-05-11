/*
 MoxMesh.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxMesh.h"
#include "MoxGeometry.h"
#include "Public/MoxMaterial.h"

namespace Mox {

Mesh::Mesh(const Mox::VertexBuffer& InVertexBuffer, const Mox::IndexBuffer& InIndexBuffer,
	const std::vector<std::tuple<Mox::SpHash, Mox::Buffer*>>& InShaderParams)
	: m_VertexBufferView(*InVertexBuffer.GetView()), m_IndexBufferView(*InIndexBuffer.GetView()),
	m_ModelMatrix(Mox::ModelMatrix(Mox::Vector3i(0, 0, 0), Mox::Vector3f(1,1,1), Mox::Vector3f(0,0,0))),
	m_Material(Mox::Material::DefaultMaterial)
{
	
	for (const std::tuple<Mox::SpHash, Mox::Buffer*>& newParam : InShaderParams)
	{
		auto [paramHash, bufPtr] = newParam;
		SetShaderParamValue(paramHash, bufPtr);
	}
}


void Mesh::SetShaderParamValue(Mox::SpHash InHash, Mox::Buffer* InBuffer)
{
	m_ShaderParameters[InHash] = InBuffer;
}

}
