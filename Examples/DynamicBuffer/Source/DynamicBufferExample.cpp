/*
 DynamicBufferExample.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "DynamicBufferExample.h"
#include <iostream>
#include <algorithm>
#include "GraphicsUtils.h"
#include "PipelineState.h"
#include "MoxGeometry.h"
#include "GraphicsAllocator.h"
#include "CommandQueue.h"
#include "Window.h"
#include "Device.h"
#include "CommandList.h"
#include "MoxMath.h"
#include "Simulator.h"
#include "Renderer.h"
#include "Entity.h"


#define PART3_SHADERS_PATH(NAME) LQUOTE(DYN_BUF_EXAMPLE_PROJ_ROOT_PATH/shaders/NAME)

int main()
{
	std::cout << "Dynamic Buffer Example" << std::endl;

	Mox::Application::Create<DynBufExampleApp>();

	Mox::Application::Get()->Initialize();

	Mox::Application::Get()->Run();

	Mox::GetDevice().ShutDown();

	// The following will trigger a breakpoint if we have some interfaces to graphics objects that were not cleaned up(leaking)!
	Mox::GetDevice().ReportLiveObjects();

}

DynBufExampleApp::DynBufExampleApp() = default;

void DynBufExampleApp::OnInitializeContent()
{


	m_VertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateEmptyResource();
	m_IndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateEmptyResource();
	m_ColorModBuffer = &Mox::AllocateDynamicBuffer();
	m_VertexBufferView = &Mox::AllocateVertexBufferView();
	m_IndexBufferView = &Mox::AllocateIndexBufferView();
	m_ColorModBufferView = &Mox::GraphicsAllocator::Get()->AllocateConstantBufferView();
	m_PipelineState = &Mox::AllocatePipelineState();

	// Load Content
	Mox::CommandList& loadContentCmdList = m_Renderer->GetCmdQueue()->GetAvailableCommandList(); // TODO renderer should not need to be exposed to the derived application

	// Upload vertex buffer data
	Mox::Resource& intermediateVertexBuffer = Mox::GraphicsAllocator::Get()->AllocateEmptyResource(); // Note: we are allocating intermediate buffer that will not be used anymore later on but will stay in memory (leak)
	Mox::GraphicsAllocator::GraphicsAllocator::Get()->AllocateBufferCommittedResource(loadContentCmdList, *m_VertexBuffer, intermediateVertexBuffer, _countof(m_VertexData), sizeof(VertexPosColor), m_VertexData);

	// Create the Vertex Buffer View associated to m_VertexBuffer
	m_VertexBufferView->ReferenceResource(*m_VertexBuffer, sizeof(m_VertexData), sizeof(VertexPosColor));

	// Upload index buffer data
	Mox::Resource& intermediateIndexBuffer = Mox::GraphicsAllocator::Get()->AllocateEmptyResource(); // Note: we are allocating intermediate buffer that will not be used anymore later on but will stay in memory (leak)

	Mox::GraphicsAllocator::GraphicsAllocator::Get()->AllocateBufferCommittedResource(loadContentCmdList, *m_IndexBuffer, intermediateIndexBuffer, _countof(m_IndexData), sizeof(unsigned short), m_IndexData);

	// Create the Index Buffer View associated to m_IndexBuffer
	m_IndexBufferView->ReferenceResource(*m_IndexBuffer, sizeof(m_IndexData), Mox::BUFFER_FORMAT::R16_UINT); // Single channel 16 bits, because WORD = unsigned short = 2 bytes = 16 bits

	// --- Shader Loading ---
	// Note: to generate the .cso file I will be using the offline method, using fxc.exe integrated in visual studio (but downloadable separately).
	// fxc command can be used by opening a developer command console in the hlsl shader folder.
	// To generate VertexShader.cso I will be using: fxc /Zi /T vs_5_1 /Fo VertexShader.cso VertexShader.hlsl
	// To generate PixelShader.cso I will be using: fxc /Zi /T ps_5_1 /Fo PixelShader.cso PixelShader.hlsl

	// Load the Vertex Shader
	Mox::Shader& vertexShader = Mox::AllocateShader(PART3_SHADERS_PATH(VertexShader.cso));
	// Load the Pixel Shader
	Mox::Shader& pixelShader = Mox::AllocateShader(PART3_SHADERS_PATH(PixelShader.cso));

	// Create the Vertex Input Layout
	Mox::PipelineState::INPUT_LAYOUT_DESC inputLayout{ {
		{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
		{"COLOR",Mox::BUFFER_FORMAT::R32G32B32_FLOAT}
	}
	};

	//Create Root Signature
	// Allow Input layout access to shader resources (in out case, the MVP matrix) 
	// and deny it to other stages (small optimization)
	Mox::PipelineState::RESOURCE_BINDER_DESC resourceBinderDesc;
	resourceBinderDesc.Flags =
		Mox::PipelineState::RESOURCE_BINDER_FLAGS::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		Mox::PipelineState::RESOURCE_BINDER_FLAGS::DENY_HULL_SHADER_ACCESS |
		Mox::PipelineState::RESOURCE_BINDER_FLAGS::DENY_DOMAIN_SHADER_ACCESS |
		Mox::PipelineState::RESOURCE_BINDER_FLAGS::DENY_GEOMETRY_SHADER_ACCESS;

	// Using a single 32-bit constant root parameter (MVP matrix) that is used by the vertex shader	
	Mox::PipelineState::RESOURCE_BINDER_PARAM mvpMatrix;
	mvpMatrix.InitAsConstants(sizeof(Eigen::Matrix4f) / 4, 0, 0, Mox::SHADER_VISIBILITY::SV_VERTEX);

	resourceBinderDesc.Params.emplace(resourceBinderDesc.Params.end(), std::move(mvpMatrix));

	// Constant buffer ColorModifierCB
	Mox::PipelineState::RESOURCE_BINDER_PARAM colorModifierParam;
	colorModifierParam.InitAsTableCBVRange(1, 0, 1, Mox::SHADER_VISIBILITY::SV_PIXEL);

	resourceBinderDesc.Params.emplace(resourceBinderDesc.Params.end(), std::move(colorModifierParam));

	// Init Root Signature Desc
	// Create Root Signature serialized blob and then the object from it
	// RTV Formats
	// Pipeline State Stream definition and fill
	Mox::PipelineState::GRAPHICS_PSO_DESC pipelineStateDesc{
		inputLayout,
		resourceBinderDesc,
		Mox::PRIMITIVE_TOPOLOGY_TYPE::PTT_TRIANGLE,
		vertexShader,
		pixelShader,
		Mox::BUFFER_FORMAT::D32_FLOAT,
		Mox::BUFFER_FORMAT::R8G8B8A8_UNORM
	};

	//Init the Pipeline State Object
	m_PipelineState->Init(pipelineStateDesc);

	// Executing command list and waiting for full execution
	m_Renderer->GetCmdQueue()->ExecuteCmdList(loadContentCmdList); // TODO renderer should not need to be exposed to the derived application

	m_Renderer->GetCmdQueue()->Flush(); // TODO renderer should not need to be exposed to the derived application

	// Initialize the Model Matrix
	m_CubeEntity = &AddEntity();
	m_CubeEntity->m_ModelMatrix = Eigen::Matrix4f::Identity();

	// Window events delegates
	m_MainWindow->OnMouseMoveDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnMouseMove>(this);
	m_MainWindow->OnMouseWheelDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnMouseWheel>(this);
	m_MainWindow->OnTypingKeyDownDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnTypingKeyPressed>(this);
	m_MainWindow->OnControlKeyDownDelegate.Add<DynBufExampleApp, &DynBufExampleApp::OnControlKeyPressed>(this);

}

void DynBufExampleApp::OnQuitApplication()
{
	Mox::Application::OnQuitApplication();

}

void DynBufExampleApp::OnMouseWheel(float InDeltaRot)
{
	// TODO this needs to be passed by message to the renderer
	//SetFov(std::max(0.2094395102f, std::min(m_MainWindowView.m_Fov -= InDeltaRot / 1200.f, 1.570796327f))); // clamping
}

void DynBufExampleApp::OnMouseMove(int32_t InX, int32_t InY)
{
	static int32_t prevX = 0, prevY = 0;
	if (m_MainWindow->IsMouseRightHold())
		OnRightMouseDrag(InX - prevX, InY - prevY);
	if (m_MainWindow->IsMouseLeftHold())
		OnLeftMouseDrag(InX - prevX, InY - prevY);

	prevX = InX; prevY = InY;
}

void DynBufExampleApp::OnLeftMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	Mox::AffineTransform<float, 3> tr;
	tr.setIdentity();
	tr.rotate(Mox::AngleAxisf(-InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), Mox::Vector3f::UnitY()))
		.rotate(Mox::AngleAxisf(-InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), Mox::Vector3f::UnitX()));

	m_CubeEntity->m_ModelMatrix = tr.matrix() * m_CubeEntity->m_ModelMatrix;
}

void DynBufExampleApp::OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
	Eigen::Transform<float, 3, Eigen::Affine> tr;
	tr.setIdentity();
	tr.translate(Eigen::Vector3f(InDeltaX / static_cast<float>(m_MainWindow->GetFrameWidth()), -InDeltaY / static_cast<float>(m_MainWindow->GetFrameHeight()), 0));

	m_CubeEntity->m_ModelMatrix = tr.matrix() * m_CubeEntity->m_ModelMatrix;
}

void DynBufExampleApp::OnTypingKeyPressed(Mox::KEYBOARD_KEY InKeyPressed)
{
	if (InKeyPressed == Mox::KEYBOARD_KEY::KEY_V)
		m_MainWindow->SetVSyncEnabled(!m_MainWindow->IsVSyncEnabled());
}

void DynBufExampleApp::OnControlKeyPressed(Mox::KEYBOARD_KEY InPressedSysKey)
{
	if (InPressedSysKey == Mox::KEYBOARD_KEY::KEY_ESC)
		m_MainWindow->Close();
}

void DynBufExampleApp::UpdateContent(float InDeltaTime)
{
	// Updating color modifier
	static float progress, counter = 0.f;
	counter = 0.5f + std::sin(progress) / 2.f; // TODO find a way (event) to transfer the buffer information to the render thread
	progress += 0.002f * InDeltaTime;

	m_ColorModBuffer->SetData(&counter, sizeof(counter), sizeof(float));
}

void DynBufExampleApp::RenderMainView(Mox::CommandList & InCmdList, const Mox::ContextView& InMainView)
{	
	// Updating cube MVP matrix
	m_MvpMatrix = InMainView.m_ProjMatrix * InMainView.m_ViewMatrix * m_CubeEntity->m_ModelMatrix;



	// Fill Command List Pipeline-related Data
	{

		InCmdList.SetPipelineStateAndResourceBinder(*m_PipelineState);

		InCmdList.SetInputAssemblerData(Mox::PRIMITIVE_TOPOLOGY::PT_TRIANGLELIST, *m_VertexBufferView, *m_IndexBufferView);

	}

	// Fill Command List Buffer Data and Draw Command
	{
		// TODO move this computation inside the simulator by feching an array of entities and retrieve the model matrix and compute the MVP
		InCmdList.SetGraphicsRootConstants(0, sizeof(Eigen::Matrix4f) / 4, m_MvpMatrix.data(), 0);

		InCmdList.StoreAndReferenceDynamicBuffer(1, *m_ColorModBuffer, *m_ColorModBufferView);

		InCmdList.DrawIndexed(_countof(m_IndexData));
	}

}
