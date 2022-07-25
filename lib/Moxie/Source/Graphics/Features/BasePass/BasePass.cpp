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
#include "MoxDrawable.h"
#include "GraphicsAllocator.h"
#include "MoxUtils.h"

namespace Mox {


	// SPH = Shader Parameter Hash
	static constexpr SpHash SPH_mvp = Mox::HashSpName("mvp");

	// Color modifier for the pixel shader
	static constexpr SpHash SPH_c_mod = Mox::HashSpName("c_mod");

	// Cubemap texture
	static constexpr SpHash SPH_cube_tex = Mox::HashSpName("cube_tex");

	enum DRAW_FEATURES : uint8_t
	{
		COLOR_MOD = 1,
		CUBE_TEX = 2
	};

	// Hashmap holding information about shader parameters that the pipeline requires for drawing.
	// This map is tied to the current pipeline state used. At the moment here in base pass we have only one,
	// but in a generic case we can have multiple possible pipeline states, which can have same shader parameters
	// placed in different root locations.
	static std::unordered_map<Mox::SpHash, Mox::ShaderParameterDefinition> m_ShaderParamDefinitionMap;

	BasePass::~BasePass()
	{
		
	}

	void BasePass::SetupPass()
	{
		// Note: in a more serious context this information should come from the shader reflection system.
		m_ShaderParamDefinitionMap = {
			{SPH_mvp, {0, "mvp", Mox::SHADER_PARAM_TYPE::CONSTANT_BUFFER, 0, 0}},
			{SPH_c_mod, {1, "c_mod", Mox::SHADER_PARAM_TYPE::CONSTANT_BUFFER, 0, 1}},
			{SPH_cube_tex, {2, "cube_tex", Mox::SHADER_PARAM_TYPE::TEXTURE, 0, 1}}
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

		// Note: at the moment we hardcode the root signature to be the same one as the dynamic buffer example

		// Mvp matrix is set as a root descriptor (size of 2 d-words) and it will be used by the vertex shader
		Mox::PipelineState::RESOURCE_BINDER_PARAM mvpMatrix;
		//mvpMatrix.InitAsConstants(sizeof(Eigen::Matrix4f) / 4, 0, 0, Mox::SHADER_VISIBILITY::SV_VERTEX); // Using a single 32-bit constant root parameter (MVP matrix) that is used by the vertex shader	

		const Mox::ShaderParameterDefinition& mvpSpInfo = m_ShaderParamDefinitionMap[SPH_mvp];
		mvpMatrix.InitAsTableCBVRange(1, mvpSpInfo.m_RegisterIndex, mvpSpInfo.m_SpaceIndex, Mox::SHADER_VISIBILITY::SV_VERTEX);

		resourceBinderDesc.Params.emplace(resourceBinderDesc.Params.end(), std::move(mvpMatrix));

		// Constant buffer ColorModifierCB
		Mox::PipelineState::RESOURCE_BINDER_PARAM colorModifierParam;
		const Mox::ShaderParameterDefinition& cModSpInfo = m_ShaderParamDefinitionMap[SPH_c_mod];
		colorModifierParam.InitAsTableCBVRange(1, cModSpInfo.m_RegisterIndex, cModSpInfo.m_SpaceIndex, Mox::SHADER_VISIBILITY::SV_PIXEL);

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

	void BasePass::ProcessRenderProxy(Mox::RenderProxy& InProxy)
	{
		// Note: for now we assume the proxy is relevant for this pass

		// Generate draw command from each mesh to later use it on draw
		for (const Mox::Drawable* curMesh : InProxy.m_Meshes)
		{	
			// featuresFiled is a bitfield intended as a cheap way to 
			// enable selective features inside the single base pass shader.
			// 
			uint32_t featuresFiled = 0;

			// ----- Generate Srv entries
			std::vector<SrvEntry> srvEntries;

			std::unordered_map<Mox::SpHash, Mox::Texture*>::const_iterator cubeTexParamValue = curMesh->m_TextureShaderParameters.find(SPH_cube_tex);

			Mox::ShaderResourceView* CubeTexSrv;

			if (cubeTexParamValue != curMesh->m_TextureShaderParameters.cend())
			{
				featuresFiled |= DRAW_FEATURES::CUBE_TEX;
				CubeTexSrv = cubeTexParamValue->second->GetResource()->GetView();
			}
			else
			{
				// Bind null descriptor
				CubeTexSrv = Mox::ShaderResourceView::GetNullCube();
			}

			srvEntries.emplace_back(m_ShaderParamDefinitionMap[SPH_cube_tex].PipelineRootIndex, CubeTexSrv);

			// ----- Generate Cbv entries
			std::vector<CbvEntry> cbvEntries;

			// We just need to store the reference to the mvp view, since the other info,
			// such as the root index and the size of data, is already known and expected by the pass.

			// Note: we are assuming to find entries for relative parameters every time
			std::unordered_map<Mox::SpHash, Mox::ConstantBuffer*>::const_iterator mvpParamValue = curMesh->m_BufferShaderParameters.find(SPH_mvp);

			Check(mvpParamValue != curMesh->m_BufferShaderParameters.cend()) // If this triggers, we are missing the MVP shader param value for this mesh

			cbvEntries.emplace_back(m_ShaderParamDefinitionMap[SPH_mvp].PipelineRootIndex, static_cast<Mox::ConstantBufferView*>(mvpParamValue->second->GetResource()->GetView()));

			std::unordered_map<Mox::SpHash, Mox::ConstantBuffer*>::const_iterator cModParamValue = curMesh->m_BufferShaderParameters.find(SPH_c_mod);

			Mox::ConstantBufferView* cmodCbv;
			if (cModParamValue != curMesh->m_BufferShaderParameters.cend())
			{
				featuresFiled |= DRAW_FEATURES::COLOR_MOD;
				cmodCbv = static_cast<Mox::ConstantBufferView*>(cModParamValue->second->GetResource()->GetView());
			}
			else
			{
				cmodCbv = Mox::ConstantBufferView::GetNull();
			}

			cbvEntries.emplace_back(m_ShaderParamDefinitionMap[SPH_c_mod].PipelineRootIndex, cmodCbv);

			m_DrawCommands.emplace_back(

				static_cast<Mox::VertexBufferView&>(*curMesh->m_VertexBuffer.GetResource()->GetView()),

				static_cast<Mox::IndexBufferView&>(*curMesh->m_IndexBuffer.GetResource()->GetView()),

				*m_PipelineState,

				cbvEntries,

				srvEntries
			
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
			for(const CbvEntry & curCbvEntry : dc.m_CbvResourceEntries)
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

			InCmdList.DrawIndexed(dc.m_IndexBufferView.GetElementsNum());
		}

	}


}
