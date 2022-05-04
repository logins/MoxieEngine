/*
 BasePass.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "BasePass.h"
#include "DrawCommand.h"
#include "GraphicsUtils.h"
#include "PipelineState.h"
#include "MoxRenderProxy.h"
#include "CommandList.h"
#include "MoxMesh.h"
#include "GraphicsAllocator.h"
#include "MoxUtils.h"

namespace Mox {


	// SPH = Shader Parameter Hash
	static const SpHash SPH_mvp = Mox::HashSpName("mvp");

	// Color modifier for the pixel shader
	static const SpHash SPH_c_mod = Mox::HashSpName("c_mod");

	BasePass::~BasePass()
	{
		
	}

	void BasePass::SetupPass()
	{
		//Create Root Signature
		// Allow Input layout access to shader resources (in out case, the MVP matrix) 
		// and deny it to other stages (small optimization)
		Mox::PipelineState::RESOURCE_BINDER_DESC resourceBinderDesc;
		resourceBinderDesc.Flags =
			Mox::PipelineState::RESOURCE_BINDER_FLAGS::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			Mox::PipelineState::RESOURCE_BINDER_FLAGS::DENY_HULL_SHADER_ACCESS |
			Mox::PipelineState::RESOURCE_BINDER_FLAGS::DENY_DOMAIN_SHADER_ACCESS |
			Mox::PipelineState::RESOURCE_BINDER_FLAGS::DENY_GEOMETRY_SHADER_ACCESS;

		// Note: at the moment we hardcode the root signature to be the same one as the dynamic buffer example

		// Mvp matrix is set as a root descriptor (size of 2 d-words) and it will be used by the vertex shader
		Mox::PipelineState::RESOURCE_BINDER_PARAM mvpMatrix;
		//mvpMatrix.InitAsConstants(sizeof(Eigen::Matrix4f) / 4, 0, 0, Mox::SHADER_VISIBILITY::SV_VERTEX); // Using a single 32-bit constant root parameter (MVP matrix) that is used by the vertex shader	
		mvpMatrix.InitAsTableCBVRange(1, 0, 0, Mox::SHADER_VISIBILITY::SV_VERTEX);

		resourceBinderDesc.Params.emplace(resourceBinderDesc.Params.end(), std::move(mvpMatrix));

		// Constant buffer ColorModifierCB
		Mox::PipelineState::RESOURCE_BINDER_PARAM colorModifierParam;
		colorModifierParam.InitAsTableCBVRange(1, 0, 1, Mox::SHADER_VISIBILITY::SV_PIXEL);

		resourceBinderDesc.Params.emplace(resourceBinderDesc.Params.end(), std::move(colorModifierParam));

		// Create the Vertex Input Layout
		Mox::PipelineState::INPUT_LAYOUT_DESC inputLayout{ {
			{"POSITION",Mox::BUFFER_FORMAT::R32G32B32_FLOAT},
			{"COLOR",Mox::BUFFER_FORMAT::R32G32B32_FLOAT}
		}
		};

		// --- Shader Loading ---
		// Note: to generate the .cso file I will be using the offline method, using fxc.exe integrated in visual studio (but downloadable separately).
		// fxc command can be used by opening a developer command console in the hlsl shader folder.
		// To generate VertexShader.cso I will be using: fxc /Zi /T vs_5_1 /Fo VertexShader.cso VertexShader.hlsl
		// To generate PixelShader.cso I will be using: fxc /Zi /T ps_5_1 /Fo PixelShader.cso PixelShader.hlsl

		// Load the Vertex Shader
		Mox::Shader& vertexShader = Mox::AllocateShader(MOX_SHADERS_PATH(BasePass_VS.cso));
		// Load the Pixel Shader
		Mox::Shader& pixelShader = Mox::AllocateShader(MOX_SHADERS_PATH(BasePass_PS.cso));

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

		m_PipelineState = &GraphicsAllocator::Get()->AllocatePipelineState();

		//Init the Pipeline State Object
		m_PipelineState->Init(pipelineStateDesc);

	}

	std::vector<CbvEntry> GenerateResourceEntriesForMesh(const Mox::Mesh& InMesh)
	{
		std::vector<CbvEntry> resourceEntries;

		// We just need to store the reference to the mvp view, since the other info,
		// such as the root index and the size of data, is already known and expected by the pass.

		// Note: we are assuming to find entries for relative parameters every time
		std::unordered_map<Mox::SpHash, Mox::ConstantBufferView*>::const_iterator mvpParamValue = InMesh.m_ShaderParameters.find(SPH_mvp);

		Check(mvpParamValue != InMesh.m_ShaderParameters.cend()) // If this triggers, we are missing the MVP shader param value for this mesh

		resourceEntries.emplace_back(0, mvpParamValue->second);

		std::unordered_map<Mox::SpHash, Mox::ConstantBufferView*>::const_iterator cModParamValue = InMesh.m_ShaderParameters.find(SPH_c_mod);

		Check(cModParamValue != InMesh.m_ShaderParameters.cend()) // If this triggers, we are missing the c_mod shader param value for this mesh

		resourceEntries.emplace_back(1, cModParamValue->second);

		return std::move(resourceEntries);
	}

	void BasePass::ProcessRenderProxy(Mox::RenderProxy& InProxy)
	{
		// Note: for now we assume the proxy is relevant for this pass

		// Generate draw command from each mesh to later use it on draw
		for (const Mox::Mesh* curMesh : InProxy.m_Meshes)
		{	
			m_DrawCommands.emplace_back(

				curMesh->m_VertexBufferView,

				curMesh->m_IndexBufferView,

				*m_PipelineState,

				GenerateResourceEntriesForMesh(*curMesh)
			
			);
		}
	}

	void BasePass::SendDrawCommands(Mox::CommandList& InCmdList)
	{
		for (const DrawCommand& dc : m_DrawCommands)
		{
			InCmdList.SetPipelineStateAndResourceBinder(*m_PipelineState);

			InCmdList.SetInputAssemblerData(Mox::PRIMITIVE_TOPOLOGY::PT_TRIANGLELIST, dc.m_VertexBufferView, dc.m_IndexBufferView);

			// Setting Resources
			
			// Checking if we have all the descriptors on gpu we need
			Mox::ConstantBufferView* curCbv;
			uint32_t rootIdx;
			for(const CbvEntry & curCbvEntry : dc.m_ResourceEntries)
			{
				auto [rootIdx, curCbv] = curCbvEntry;
				// If the view is Gpu allocated, just reference the view in the command list
				if (curCbv->IsGpuAllocated())
				{
					InCmdList.ReferenceCBV(rootIdx, *curCbv);
				}
				else
				// Otherwise we stage it to dynamically allocate one in the position it belongs
				{
					InCmdList.StageDynamicCbv(rootIdx, *curCbv);
				}
			}
			// Pushing all the staged descriptors to Gpu and assigning them to the root signature
			InCmdList.CommitStagedViews();

			//InCmdList.SetGraphicsRootConstants(0, sizeof(Eigen::Matrix4f) / 4, std::get<1>(dc.m_ResourceEntries[0])->GetResource().GetData(), 0);

			// Note: base pass should not be responsible of updating the dynamic buffer like it happens here, 
			// because that should happen at the start of the frame when we update values coming from the simulation thread, 
			// so this needs to change in just "reference buffer" and the "store" to be done early at the beginning of the whole frame,
			// just after updating proxies.
			//InCmdList.StoreAndReferenceDynamicBuffer(1, *m_ColorModBuffer, *m_ColorModBufferView);

			//InCmdList.ReferenceCBV(1, *std::get<1>(dc.m_ResourceEntries[1]));

			InCmdList.DrawIndexed(dc.m_IndexBufferView.GetIB().GetElementsNum());
		}

	}


}
