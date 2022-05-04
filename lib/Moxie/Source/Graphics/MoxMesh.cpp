/*
 MoxMesh.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "MoxMesh.h"
#include "MoxGeometry.h"
#include "Public/MoxMaterial.h"

namespace Mox {

Mesh::Mesh(const Mox::Vector3i& InPosition, const Mox::Vector3f& InRotation, Mox::VertexBufferView& InVertexBuffer, Mox::IndexBufferView& InIndexBuffer)
	: m_VertexBufferView(InVertexBuffer), m_IndexBufferView(InIndexBuffer),
	m_ModelMatrix(Mox::ModelMatrix(InPosition, Mox::Vector3f(1,1,1), Mox::Vector3f(0,0,0))),
	m_Material(Mox::Material::DefaultMaterial)
{
	
	// Creating the buffer (and relative cbv) for the model matrix
	Mox::Buffer&  mvpBuffer = Mox::AllocateDynamicBuffer(sizeof(m_ModelMatrix));

	const Eigen::Vector3f eyePosition = Eigen::Vector3f(0, 0, -10);
	const Eigen::Vector3f focusPoint = Eigen::Vector3f(0, 0, 0);
	const Eigen::Vector3f upDirection = Eigen::Vector3f(0, 1, 0);
	Mox::Matrix4f viewMatrix = Mox::LookAt(eyePosition, focusPoint, upDirection);
												// z_min z_max aspect_ratio fov
	Mox::Matrix4f projMatrix = Mox::Perspective(0.1f, 100.f, 1.3333f, 0.7853981634f);

	Mox::Matrix4f mvpValue = projMatrix * viewMatrix * m_ModelMatrix;

	mvpBuffer.SetData(mvpValue.data(), sizeof(mvpValue));

	//mvpBuffer.SetData_RenderThread(mvpValue.data(), sizeof(mvpValue));

	// TODO test
	//Mox::ConstantBufferView& mvpTest = Mox::AllocateConstantBufferView(mvpBuffer);

	// Setting the relative shader parameter
	//SetShaderParamValue(Mox::HashSpName("mvp"), *mvpBuffer.GetView());
	SetShaderParamValue(Mox::HashSpName("mvp"), mvpBuffer.GetView());


	// Creating buffer for the color mod
	Mox::Buffer& colorModBuffer = Mox::AllocateDynamicBuffer(sizeof(float));
	float colorMod = .5f;
	colorModBuffer.SetData(&colorMod, sizeof(float));
	// Setting the relative shader parameter
	SetShaderParamValue(Mox::HashSpName("c_mod"), colorModBuffer.GetView());

}


void Mesh::SetShaderParamValue(Mox::SpHash InHash, Mox::ConstantBufferView* InCbv)
{
	m_ShaderParameters[InHash] = InCbv;
}

}
