/*
 D3D12PipelineState.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "D3D12PipelineState.h"
#include "D3D12UtilsInternal.h"
#include "D3D12Device.h"
#include "MoxUtils.h"
#include "D3D12MoxUtils.h"

#define FAILED(hr)      (((HRESULT)(hr)) < 0)

namespace Mox{ 

	using namespace Mox;

	D3D12_ROOT_SIGNATURE_FLAGS D3D12PipelineState::TransformResourceBinderFlags(RESOURCE_BINDER_FLAGS InResourceBinderFlags)
	{
		if (InResourceBinderFlags == RESOURCE_BINDER_FLAGS::NONE)
			return D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE;

		D3D12_ROOT_SIGNATURE_FLAGS returnFlags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE;

		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::ALLOW_STREAM_OUTPUT)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::DENY_DOMAIN_SHADER_ACCESS)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::DENY_GEOMETRY_SHADER_ACCESS)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::DENY_HULL_SHADER_ACCESS)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::DENY_PIXEL_SHADER_ACCESS)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		if (InResourceBinderFlags & RESOURCE_BINDER_FLAGS::DENY_VERTEX_SHADER_ACCESS)
			returnFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

		return returnFlags;
	}

	D3D12_INPUT_ELEMENT_DESC D3D12PipelineState::TransformInputLayoutElement(INPUT_LAYOUT_DESC::LayoutElement& InLayoutElementDesc)
	{
		return { InLayoutElementDesc.m_Name.c_str(), 0, Mox::BufferFormatToD3D12(InLayoutElementDesc.m_Format), 
			0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION::D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	}

	CD3DX12_STATIC_SAMPLER_DESC D3D12PipelineState::TransformStaticSamplerElement(Mox::StaticSampler& InLayoutElementDesc)
	{
		return CD3DX12_STATIC_SAMPLER_DESC(InLayoutElementDesc.m_ShaderRegister, Mox::SampleFilterToD3D12(InLayoutElementDesc.m_Filter), 
			Mox::TextureAddressModeToD3D12(InLayoutElementDesc.m_AddressU), Mox::TextureAddressModeToD3D12(InLayoutElementDesc.m_AddressV), Mox::TextureAddressModeToD3D12(InLayoutElementDesc.m_AddressW));
	}

	CD3DX12_ROOT_PARAMETER1 D3D12PipelineState::TransformResourceBinderParams(RESOURCE_BINDER_PARAM& InResourceBinderParam, std::vector<D3D12_DESCRIPTOR_RANGE1>& OutDescRanges)
	{
		CD3DX12_ROOT_PARAMETER1 returnParam = {};
		switch (InResourceBinderParam.ResourceType)
		{
		case  RESOURCE_BINDER_PARAM::RESOURCE_TYPE::CONSTANTS:
		{
			returnParam.InitAsConstants(InResourceBinderParam.Num32BitValues, InResourceBinderParam.ShaderRegister,
			InResourceBinderParam.RegisterSpace, TransformShaderVisibility(InResourceBinderParam.shaderVisibility));
			break;
		}
		case RESOURCE_BINDER_PARAM::RESOURCE_TYPE::CBV_RANGE:
		{
			// Note: we are assuming only 1 descriptor range per descriptor table
			D3D12_DESCRIPTOR_RANGE1 descRange;
			descRange.BaseShaderRegister = InResourceBinderParam.ShaderRegister;
			descRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
			descRange.NumDescriptors = InResourceBinderParam.NumDescriptors;
			descRange.OffsetInDescriptorsFromTableStart = 0;
			descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			descRange.RegisterSpace = InResourceBinderParam.RegisterSpace;

			OutDescRanges.push_back(std::move(descRange));

			returnParam.InitAsDescriptorTable(1, &OutDescRanges.back(), TransformShaderVisibility(InResourceBinderParam.shaderVisibility));
			break;
		}
		case RESOURCE_BINDER_PARAM::RESOURCE_TYPE::SRV_RANGE:
		{
			// Note: we are assuming only 1 descriptor range per descriptor table
			D3D12_DESCRIPTOR_RANGE1 descRange;
			descRange.BaseShaderRegister = InResourceBinderParam.ShaderRegister;
			descRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE; // TODO need to be able to pass flags with root params
			descRange.NumDescriptors = InResourceBinderParam.NumDescriptors;
			descRange.OffsetInDescriptorsFromTableStart = 0;
			descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descRange.RegisterSpace = InResourceBinderParam.RegisterSpace;

			OutDescRanges.push_back(std::move(descRange));

			returnParam.InitAsDescriptorTable(1, &OutDescRanges.back(), TransformShaderVisibility(InResourceBinderParam.shaderVisibility));
			break;
		}
		case RESOURCE_BINDER_PARAM::RESOURCE_TYPE::UAV_RANGE:
		{
			// Note: we are assuming only 1 descriptor range per descriptor table
			D3D12_DESCRIPTOR_RANGE1 descRange;
			descRange.BaseShaderRegister = InResourceBinderParam.ShaderRegister;
			descRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE; // TODO need to be able to pass flags with root params
			descRange.NumDescriptors = InResourceBinderParam.NumDescriptors;
			descRange.OffsetInDescriptorsFromTableStart = 0;
			descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			descRange.RegisterSpace = InResourceBinderParam.RegisterSpace;

			OutDescRanges.push_back(std::move(descRange));

			returnParam.InitAsDescriptorTable(1, &OutDescRanges.back(), TransformShaderVisibility(InResourceBinderParam.shaderVisibility));
			break;
		}
		default:
			StopForFail("Root Param transformation not implemented yet!")
			break;
		}
		return returnParam;
	}

	D3D12_SHADER_VISIBILITY D3D12PipelineState::TransformShaderVisibility(SHADER_VISIBILITY shaderVisibility)
	{
		switch (shaderVisibility)
		{
		case Mox::SHADER_VISIBILITY::SV_ALL: return D3D12_SHADER_VISIBILITY_ALL;
		case Mox::SHADER_VISIBILITY::SV_VERTEX: return D3D12_SHADER_VISIBILITY_VERTEX;
		case Mox::SHADER_VISIBILITY::SV_HULL: return D3D12_SHADER_VISIBILITY_HULL;
		case Mox::SHADER_VISIBILITY::SV_DOMAIN: return D3D12_SHADER_VISIBILITY_DOMAIN;
		case Mox::SHADER_VISIBILITY::SV_GEOMETRY: return D3D12_SHADER_VISIBILITY_GEOMETRY;
		case Mox::SHADER_VISIBILITY::SV_PIXEL: return D3D12_SHADER_VISIBILITY_PIXEL;
		}
		StopForFail("Shader Visibility undefined.");
		return D3D12_SHADER_VISIBILITY_ALL;
	}

	// Init for a Graphics PSO
	void D3D12PipelineState::Init(GRAPHICS_PSO_DESC& InPipelineStateDesc)
	{
		m_IsGraphicsPSO = true;

		// Generate D3D12 Input Layout
		std::vector<INPUT_LAYOUT_DESC::LayoutElement>& agnosticLayoutElements = InPipelineStateDesc.InputLayoutDesc.LayoutElements;
		std::vector<D3D12_INPUT_ELEMENT_DESC> layoutElements;
		layoutElements.reserve(agnosticLayoutElements.size());
		// Convert platform-agnostic parameters into d3d12 specific
		std::transform(agnosticLayoutElements.begin(), agnosticLayoutElements.end(), std::back_inserter(layoutElements), &D3D12PipelineState::TransformInputLayoutElement);

		Microsoft::WRL::ComPtr<ID3D12Device2> d3d12GraphicsDevice = static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner();
		GenerateRootSignature(d3d12GraphicsDevice, InPipelineStateDesc.ResourceBinderDesc);

		//RTV Formats
		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets = 1; // Note: we are supporting only 1 render target at the moment
		rtvFormats.RTFormats[0] = Mox::BufferFormatToD3D12(InPipelineStateDesc.RTFormat);// Note: texel data of the render target is 4x 8bit channels of range [0,1]

		// Pipeline State Stream definition and fill
		// Note: for now we assume PipelineStateStreamType to be always the same
		struct PipelineStateStreamType {
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopology;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_VS VertexShader;
			CD3DX12_PIPELINE_STATE_STREAM_PS PixelShader;
			CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
			CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
		} pipelineStateStream;

		pipelineStateStream.pRootSignature = m_RootSignature.Get();
	
		pipelineStateStream.PrimitiveTopology = Mox::PrimitiveTopologyTypeToD3D12(InPipelineStateDesc.TopologyType);
		pipelineStateStream.InputLayout = { &layoutElements[0], static_cast<UINT>(layoutElements.size()) };
		pipelineStateStream.VertexShader = CD3DX12_SHADER_BYTECODE(static_cast<Mox::D3D12Shader&>(InPipelineStateDesc.VertexShader).m_ShaderBlob.Get());
		CD3DX12_RASTERIZER_DESC rasterDesc = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
		rasterDesc.FrontCounterClockwise = InPipelineStateDesc.RenderBackfaces;
		pipelineStateStream.Rasterizer = rasterDesc;
		pipelineStateStream.PixelShader = CD3DX12_SHADER_BYTECODE(static_cast<Mox::D3D12Shader&>(InPipelineStateDesc.PixelShader).m_ShaderBlob.Get());
		pipelineStateStream.DSVFormat = Mox::BufferFormatToD3D12(InPipelineStateDesc.DSFormat);
		pipelineStateStream.RTVFormats = rtvFormats;
		CD3DX12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
		blendDesc.RenderTarget[0] = D3D12_RENDER_TARGET_BLEND_DESC{
			  true,//BOOL           BlendEnable;
			  false, //BOOL           LogicOpEnable;
			  D3D12_BLEND_SRC_ALPHA,//D3D12_BLEND    SrcBlend;
			  D3D12_BLEND_INV_SRC_ALPHA, //D3D12_BLEND    DestBlend;
			  D3D12_BLEND_OP_ADD,//D3D12_BLEND_OP BlendOp;
			  D3D12_BLEND_ONE, //D3D12_BLEND    SrcBlendAlpha;
			  D3D12_BLEND_ONE, //D3D12_BLEND    DestBlendAlpha;
			  D3D12_BLEND_OP_ADD, //D3D12_BLEND_OP BlendOpAlpha;
			  D3D12_LOGIC_OP_CLEAR, //D3D12_LOGIC_OP LogicOp;
			  D3D12_COLOR_WRITE_ENABLE_ALL//UINT8          RenderTargetWriteMask;
		};
		pipelineStateStream.BlendDesc = blendDesc;

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(pipelineStateStream), &pipelineStateStream
		};
		Mox::ThrowIfFailed(d3d12GraphicsDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

		m_IsInitialized = true;
	}

	// Init for a Compute PSO
	void D3D12PipelineState::Init(COMPUTE_PSO_DESC& InPipelineStateDesc)
	{
		Microsoft::WRL::ComPtr<ID3D12Device2> d3d12GraphicsDevice = static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner();

		// Root signature generation
		GenerateRootSignature(d3d12GraphicsDevice, InPipelineStateDesc.ResourceBinderDesc);

		// Pipeline State Stream definition and fill
		// Note: for now we assume PipelineStateStreamType to be always the same
		struct PipelineStateStreamType {
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_CS ComputeShader;
		} pipelineStateStream;

		pipelineStateStream.RootSignature = m_RootSignature.Get();
		pipelineStateStream.ComputeShader = CD3DX12_SHADER_BYTECODE(static_cast<Mox::D3D12Shader&>(InPipelineStateDesc.ComputeShader).m_ShaderBlob.Get());

		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(pipelineStateStream), &pipelineStateStream
		};
		Mox::ThrowIfFailed(d3d12GraphicsDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

		m_IsInitialized = true;
	}

	void D3D12PipelineState::GenerateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device2> d3d12GraphicsDevice, RESOURCE_BINDER_DESC& InResourceBinder)
	{
		// Create Root Signature
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(d3d12GraphicsDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		// Allow Input layout access to shader resources
		// and deny it to other stages (small optimization)
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = TransformResourceBinderFlags(InResourceBinder.Flags);

		// --- Root Parameters ---
		std::vector<RESOURCE_BINDER_PARAM>& agnosticRootParameters = InResourceBinder.Params;
		std::vector<CD3DX12_ROOT_PARAMETER1>& rootParameters = m_RootSignatureInfo.rootParameters;
		rootParameters.reserve(agnosticRootParameters.size());

		// We cannot use std::transform here because in the case of descriptor tables for example, we also need to preserve the generated D3D12_DESCRIPTOR_RANGE1 objects other than the root parameters
		m_RootSignatureInfo.m_DescRanges.reserve(10);
		for (uint32_t i = 0; i < agnosticRootParameters.size(); i++)
		{
			rootParameters.push_back(TransformResourceBinderParams(agnosticRootParameters[i], m_RootSignatureInfo.m_DescRanges));
		}

		// --- Static Samplers ---
		std::vector<StaticSampler>& agnosticStaticSamplers = InResourceBinder.StaticSamplers;
		std::vector<CD3DX12_STATIC_SAMPLER_DESC>& staticSamplers = m_RootSignatureInfo.m_StaticSamplers;
		rootParameters.reserve(agnosticStaticSamplers.size());
		std::transform(agnosticStaticSamplers.begin(), agnosticStaticSamplers.end(), std::back_inserter(staticSamplers), &D3D12PipelineState::TransformStaticSamplerElement);

		// Init Root Signature Desc
		m_RootSignatureInfo.rootSignatureDesc.Init_1_1(rootParameters.size(), rootParameters.data(), staticSamplers.size(), staticSamplers.data(), rootSignatureFlags);
		// Create Root Signature serialized blob and then the object from it
		m_RootSignature = Mox::SerializeAndCreateRootSignature(d3d12GraphicsDevice, &m_RootSignatureInfo.rootSignatureDesc, featureData.HighestVersion);
	}

	uint32_t D3D12PipelineState::GenerateRootTableBitMask()
	{
		uint32_t outRootTableMask = 0;
		for (size_t i = 0; i < m_RootSignatureInfo.rootParameters.size(); i++)
		{
			if (m_RootSignatureInfo.rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				outRootTableMask |= (1 << i); // If root table, setting the corresponding bit at position i to 1
		}
		return outRootTableMask;
	}

	uint32_t D3D12PipelineState::GetRootDescriptorsNumAtIndex(uint32_t InRootIndex)
	{
		Check(m_RootSignatureInfo.rootParameters[InRootIndex].ParameterType == D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

		// Note: we are assuming that we store a single descriptor range for each desc table
		return m_RootSignatureInfo.rootParameters[InRootIndex].DescriptorTable.pDescriptorRanges->NumDescriptors; // TODO continue here, the reported NumDescriptors is wrong!
	}

}