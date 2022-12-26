/*
 MoxMesh.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxDrawable.h"
#include "MoxGeometry.h"
#include "Public/MoxMaterial.h"

namespace Mox {

Drawable::Drawable(const Mox::DrawableCreationInfo& InCreationInfo)
	: m_VertexBuffer(*InCreationInfo.m_VertexBuffer), m_IndexBuffer(*InCreationInfo.m_IndexBuffer),
	m_RenderBackfaces(InCreationInfo.m_RenderBackfaces), m_Material(Mox::Material::DefaultMaterial)
{
	for (const std::tuple<Mox::SpHash, Mox::ConstantBuffer*>& newCbParam : InCreationInfo.m_BufferShaderParameters)
	{
		auto [paramHash, bufPtr] = newCbParam;
		SetCbShaderParamValue(paramHash, bufPtr);
	}
	for (const std::tuple<Mox::SpHash, Mox::Texture*>& newTexParam : InCreationInfo.m_TextureShaderParameters)
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
