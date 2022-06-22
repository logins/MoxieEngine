/*
 TexturesExample.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "TexturesExample.h"
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
#include "MoxEntity.h"
#include "Simulator.h"
#include "Renderer.h"


//#define TEXTURES_EXAMPLE_SHADERS_PATH(NAME) LQUOTE(TEXTURES_EXAMPLE_PROJ_ROOT_PATH/shaders/NAME)
#define TEXTURES_EXAMPLE_CONTENT_PATH(NAME) LQUOTE(TEXTURES_EXAMPLE_PROJ_ROOT_PATH/Content/NAME)


int main()
{
	std::cout << "Moxie Example Application: Texture Usage!" << std::endl;

	Mox::Application::Create<TexturesExampleApplication>();

	Mox::Application::Get()->Initialize();

	Mox::Application::Get()->Run();

	Mox::GetDevice().ShutDown();

	// The following will trigger a breakpoint if we have some interfaces to graphics objects that were not cleaned up(leaking)!
	Mox::GetDevice().ReportLiveObjects();
}


void TexturesExampleApplication::OnInitializeContent()
{
	// Load Content

	m_VertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateVertexBuffer(m_VertexData, sizeof(VertexPosColor), sizeof(m_VertexData)); // TODO can we deduce these last two elements from compiler??
	m_IndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateIndexBuffer(m_IndexData, sizeof(unsigned short), sizeof(m_IndexData));

	Mox::TextureDesc TexDesc;
	void* TexData;

	m_Cubemap = std::make_unique<Mox::Texture>(TEXTURES_EXAMPLE_CONTENT_PATH(CubeMap.dds));

	/*

	// Load Content
	Mox::CommandList& loadContentCmdList = m_Renderer->GetCmdQueue()->GetAvailableCommandList();

	// --- Vertex Buffer ---
	size_t vertexDataSize = sizeof(VertexPosColor) * _countof(m_VertexData);

	m_VertexBuffer = &Mox::GraphicsAllocator::Get()->AllocateBufferResource(vertexDataSize, Mox::RESOURCE_HEAP_TYPE::DEFAULT, Mox::RESOURCE_STATE::COPY_DEST); // Note: this is this supposed to be created as COPY_DEST and later changing state to read
	// Note: we are allocating intermediate buffer that will not be used anymore later on but will stay in memory (leak)
	Mox::Buffer& intermediateVertexBuffer = Mox::GraphicsAllocator::Get()->AllocateBufferResource(vertexDataSize, Mox::RESOURCE_HEAP_TYPE::UPLOAD, Mox::RESOURCE_STATE::GEN_READ);

	loadContentCmdList.UploadBufferData(*m_VertexBuffer, intermediateVertexBuffer, m_VertexData, vertexDataSize);

	// Create the Vertex Buffer View associated to m_VertexBuffer
	m_VertexBufferView = &Mox::GraphicsAllocator::Get()->AllocateVertexBufferView();
	m_VertexBufferView->ReferenceResource(*m_VertexBuffer, sizeof(m_VertexData), sizeof(VertexPosColor));

	// --- Index Buffer ---
	size_t indexDataSize = sizeof(unsigned short) * _countof(m_IndexData);

	m_IndexBuffer = &Mox::GraphicsAllocator::Get()->AllocateBufferResource(indexDataSize, Mox::RESOURCE_HEAP_TYPE::DEFAULT, Mox::RESOURCE_STATE::COPY_DEST);
	// Note: we are allocating intermediate buffer that will not be used anymore later on but will stay in memory (leak)
	Mox::Buffer& intermediateIndexBuffer = Mox::GraphicsAllocator::Get()->AllocateBufferResource(indexDataSize, Mox::RESOURCE_HEAP_TYPE::UPLOAD, Mox::RESOURCE_STATE::GEN_READ);

	loadContentCmdList.UploadBufferData(*m_IndexBuffer, intermediateIndexBuffer, m_IndexData, indexDataSize);

	// Create the Index Buffer View associated to m_IndexBuffer
	m_IndexBufferView = &Mox::GraphicsAllocator::Get()->AllocateIndexBufferView();
	m_IndexBufferView->ReferenceResource(*m_IndexBuffer, sizeof(m_IndexData), Mox::BUFFER_FORMAT::R16_UINT); // Single channel 16 bits, because WORD = unsigned short = 2 bytes = 16 bits

	// --- Shader Loading ---
	// Note: to generate the .cso file I will be using the offline method, using fxc.exe integrated in visual studio (but downloadable separately).
	// fxc command can be used by opening a developer command console in the hlsl shader folder.
	// To generate VertexShader.cso I will be using: fxc /Zi /T vs_5_1 /Fo VertexShader.cso VertexShader.hlsl
	// To generate PixelShader.cso I will be using: fxc /Zi /T ps_5_1 /Fo PixelShader.cso PixelShader.hlsl

	// Load the Vertex Shader
	Mox::Shader& vertexShader = Mox::AllocateShader(TEXTURES_EXAMPLE_SHADERS_PATH(VertexShader.cso));
	// Load the Pixel Shader
	Mox::Shader& pixelShader = Mox::AllocateShader(TEXTURES_EXAMPLE_SHADERS_PATH(PixelShader.cso));

	// --- Pipeline State ---
	// Create the Vertex Input Layout
	Mox::PipelineState::INPUT_LAYOUT_DESC inputLayout{ {
		{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
		{"TEX_COORDS",Mox::BUFFER_FORMAT::R32G32B32_FLOAT}
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

	// Cubemap loading
	// The file was made using the handy ATI CubeMapGen (discontinued) available at https://gpuopen.com/archived/cubemapgen/

	m_Cubemap = &Mox::GraphicsAllocator::Get()->AllocateTextureFromFile(TEXTURES_EXAMPLE_CONTENT_PATH(CubeMap.dds), Mox::TEXTURE_FILE_FORMAT::DDS,
		5, Mox::RESOURCE_FLAGS::ALLOW_UNORDERED_ACCESS); // Force mips to 5. The loaded file contains 1 mip, and we are going to generate the other 4 mip levels later on
	// Uploading cubemap data in GPU
	// Note: we are allocating intermediate buffer that will not be used anymore later on but will stay in memory (leak)
	Mox::Buffer& intermediateCubemapBuffer = Mox::GraphicsAllocator::Get()->AllocateBufferResource(m_Cubemap->GetGPUSize(), Mox::RESOURCE_HEAP_TYPE::UPLOAD, Mox::RESOURCE_STATE::GEN_READ);

	m_Cubemap->UploadToGPU(loadContentCmdList, intermediateCubemapBuffer);

	// SRV referencing the cubemap
	m_CubemapView = &Mox::GraphicsAllocator::Get()->AllocateShaderResourceView(*m_Cubemap);

	loadContentCmdList.UploadViewToGPU(*m_CubemapView);

	// Root parameter for the cubemap
	Mox::PipelineState::RESOURCE_BINDER_PARAM cubemapParam;
	cubemapParam.InitAsTableSRVRange(1, 0, 1, Mox::SHADER_VISIBILITY::SV_PIXEL);

	resourceBinderDesc.Params.emplace_back(std::move(cubemapParam));

	// Sampler for the cubemap
	resourceBinderDesc.StaticSamplers.emplace_back(0, Mox::SAMPLE_FILTER_TYPE::LINEAR);

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

	m_PipelineState = &Mox::AllocatePipelineState();

	//Init the Pipeline State Object
	m_PipelineState->Init(pipelineStateDesc);

	// --- MIPS GENERATION ---

	// Creating 4 UAVs to target 1 mip each starting from level 1
	// Note: the Tex 2D Array will refer to the selected mip array of faces!
	Mox::UnorderedAccessView& cubeMipView1 = Mox::GraphicsAllocator::Get()->AllocateUavTex2DArray(*m_Cubemap, 6, 1);
	Mox::UnorderedAccessView& cubeMipView2 = Mox::GraphicsAllocator::Get()->AllocateUavTex2DArray(*m_Cubemap, 6, 2);
	Mox::UnorderedAccessView& cubeMipView3 = Mox::GraphicsAllocator::Get()->AllocateUavTex2DArray(*m_Cubemap, 6, 3);
	Mox::UnorderedAccessView& cubeMipView4 = Mox::GraphicsAllocator::Get()->AllocateUavTex2DArray(*m_Cubemap, 6, 4);
	// We also need an SRV Tex2D Array that points to our initially loaded mip0 cubemap
	// We can use an SRV because we are just reading from it
	Mox::ShaderResourceView& inputCubeFacesView = Mox::GraphicsAllocator::Get()->AllocateSrvTex2DArray(*m_Cubemap, 6);

	// Views need to be uploaded to GPU
	loadContentCmdList.UploadViewToGPU(inputCubeFacesView);
	loadContentCmdList.UploadUavToGpu(cubeMipView1);
	loadContentCmdList.UploadUavToGpu(cubeMipView2);
	loadContentCmdList.UploadUavToGpu(cubeMipView3);
	loadContentCmdList.UploadUavToGpu(cubeMipView4); // Note: there is room for optimization in this process since we could allocate an entire range of Uavs altogether

	//Create Root Signature
	Mox::PipelineState::RESOURCE_BINDER_DESC resourceBinderDesc2;
	// Root Signature
	// We are gonna use a root constant to pass the GenerateMipsCB to the shader
	Mox::PipelineState::RESOURCE_BINDER_PARAM generateMipsCbParam;
	generateMipsCbParam.InitAsConstants(sizeof(GenerateMipsCB) / 4, 0, 0);
	resourceBinderDesc2.Params.emplace_back(std::move(generateMipsCbParam));
	// Root parameter for the cubemap
	// We first need to have one SRV range that will be used by the input cube faces
	Mox::PipelineState::RESOURCE_BINDER_PARAM inputCubeFacesParam;
	inputCubeFacesParam.InitAsTableSRVRange(1, 0);
	resourceBinderDesc2.Params.emplace_back(std::move(inputCubeFacesParam));
	// Root parameter for output mips
	Mox::PipelineState::RESOURCE_BINDER_PARAM cubeMipViewsParam;
	cubeMipViewsParam.InitAsTableUAVRange(4, 0);
	resourceBinderDesc2.Params.emplace_back(std::move(cubeMipViewsParam));
	// Sampler for the cubemap
	resourceBinderDesc2.StaticSamplers.emplace_back(0, Mox::SAMPLE_FILTER_TYPE::LINEAR, Mox::TEXTURE_ADDRESS_MODE::CLAMP);
	// Loading compute shader
	// To generate VertexShader.cso I will be using: fxc /Zi /T cs_5_1 /Fo GenerateCubeMips_CS.cso GenerateCubeMips_CS.hlsl
	Mox::Shader& computeShader = Mox::AllocateShader(TEXTURES_EXAMPLE_SHADERS_PATH(GenerateCubeMips_CS.cso));
	// Init Root Signature and PSO
	Mox::PipelineState::COMPUTE_PSO_DESC pipelineStateDesc2{
	resourceBinderDesc2,
	computeShader
	};

	// Used to generate mips
	Mox::PipelineState& m_PipelineState2 = Mox::AllocatePipelineState();

	//Init the Pipeline State Object
	m_PipelineState2.Init(pipelineStateDesc2);
	// Set the PSO+RS
	loadContentCmdList.SetPipelineStateAndResourceBinder(m_PipelineState2);
	// Set resource binding
	loadContentCmdList.ReferenceComputeTable(1, inputCubeFacesView);
	// Note: This descriptor table is expecting a range of 4 descriptors, 
	// but we know that cubeMipView1 in GPU memory will be followed by 2,3 and 4 because we instantiated them one after the other
	loadContentCmdList.ReferenceComputeTable(2, cubeMipView1);

	// Since our compute shader handles portions of 8 by 8 texels for each thread group, 
	// the number of thread groups, in X and Y dimensions, in our dispatch will be the size of the mip 1 (so half the size of mip0), aligned by 8 and then divided by 8.
	uint32_t mip1SizeAligned = Mox::Align(m_Cubemap->GetWidth() / 2, 8);

	GenerateMipsCB genMipsCB; genMipsCB.Mip1Size = Eigen::Vector2f(mip1SizeAligned, mip1SizeAligned);

	loadContentCmdList.SetComputeRootConstants(0, sizeof(GenerateMipsCB) / 4, &genMipsCB, 0);

	// In the Z dimension the number of thread groups will be 6, because we are going to repeat the work on X and Y for each of the 6 cube faces.
	loadContentCmdList.Dispatch(mip1SizeAligned / 8, mip1SizeAligned / 8, 6);

	// Executing command list and waiting for full execution
	m_Renderer->GetCmdQueue()->ExecuteCmdList(loadContentCmdList);

	m_Renderer->GetCmdQueue()->Flush(); // Note: Flushing operations on the command queue here will ensure that all the operations made on resources by the loadContentCmdList finished executing!

	// --- MIPS GENERATION ENDS ---

	// Initialize the Model Matrix
	m_CubeEntity = &AddEntity();
	m_CubeEntity->m_ModelMatrix = Eigen::Matrix4f::Identity();
*/
	// Window events delegates
	m_MainWindow->OnMouseMoveDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnMouseMove>(this);
	m_MainWindow->OnMouseWheelDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnMouseWheel>(this);
	m_MainWindow->OnTypingKeyDownDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnTypingKeyPressed>(this);
	m_MainWindow->OnControlKeyDownDelegate.Add<TexturesExampleApplication, &TexturesExampleApplication::OnControlKeyPressed>(this);
}

void TexturesExampleApplication::OnMouseWheel(float InDeltaRot)
{
}

void TexturesExampleApplication::OnMouseMove(int32_t InX, int32_t InY)
{
	static int32_t prevX = 0, prevY = 0;
	if (m_MainWindow->IsMouseRightHold())
		OnRightMouseDrag(InX - prevX, InY - prevY);
	if (m_MainWindow->IsMouseLeftHold())
		OnLeftMouseDrag(InX - prevX, InY - prevY);

	prevX = InX; prevY = InY;
}

void TexturesExampleApplication::OnLeftMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
}

void TexturesExampleApplication::OnRightMouseDrag(int32_t InDeltaX, int32_t InDeltaY)
{
}

void TexturesExampleApplication::OnTypingKeyPressed(Mox::KEYBOARD_KEY InKeyPressed)
{
	if (InKeyPressed == Mox::KEYBOARD_KEY::KEY_V)
		m_MainWindow->SetVSyncEnabled(!m_MainWindow->IsVSyncEnabled());
}

void TexturesExampleApplication::OnControlKeyPressed(Mox::KEYBOARD_KEY InPressedSysKey)
{
	if (InPressedSysKey == Mox::KEYBOARD_KEY::KEY_ESC)
		m_MainWindow->Close();
}

void TexturesExampleApplication::UpdateContent(float InDeltaTime)
{

	// Updating MVP matrix


}