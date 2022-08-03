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
	const std::vector<std::tuple<Mox::SpHash, Mox::ConstantBuffer*>>& InBufferShaderParams,
	const std::vector<std::tuple<Mox::SpHash, Mox::Texture*>>& InTextureShaderParams)
	: m_VertexBuffer(InVertexBuffer), m_IndexBuffer(InIndexBuffer),
	m_Material(Mox::Material::DefaultMaterial)
{
	
	for (const std::tuple<Mox::SpHash, Mox::ConstantBuffer*>& newCbParam : InBufferShaderParams)
	{
		auto [paramHash, bufPtr] = newCbParam;
		SetCbShaderParamValue(paramHash, bufPtr);
	}
	for (const std::tuple<Mox::SpHash, Mox::Texture*>& newTexParam : InTextureShaderParams)
	{
		auto [paramHash, texPtr] = newTexParam;
		SetTexShaderParamValue(paramHash, texPtr);
	}
}

Drawable::~Drawable() = default;

void Drawable::SetCbShaderParamValue(Mox::SpHash InHash, Mox::ConstantBuffer* InBuffer)
{
	m_BufferShaderParameters[InHash] = InBuffer;
}

void Drawable::SetTexShaderParamValue(Mox::SpHash InHash, Mox::Texture* InTexture)
{
	m_TextureShaderParameters[InHash] = InTexture;
}

}
