/*
 MoxMesh.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxDrawable.h"
#include "MoxGeometry.h"
#include "Public/MoxMaterial.h"

namespace Mox {

Drawable::Drawable(Mox::VertexBuffer& InVertexBuffer, Mox::IndexBuffer& InIndexBuffer,
	const std::vector<std::tuple<Mox::SpHash, Mox::ConstantBuffer*>>& InShaderParams)
	: m_VertexBuffer(InVertexBuffer), m_IndexBuffer(InIndexBuffer),
	m_Material(Mox::Material::DefaultMaterial)
{
	
	for (const std::tuple<Mox::SpHash, Mox::ConstantBuffer*>& newParam : InShaderParams)
	{
		auto [paramHash, bufPtr] = newParam;
		SetShaderParamValue(paramHash, bufPtr);
	}
}

Drawable::~Drawable() = default;

void Drawable::SetShaderParamValue(Mox::SpHash InHash, Mox::ConstantBuffer* InBuffer)
{
	m_ShaderParameters[InHash] = InBuffer;
}

}
