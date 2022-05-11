/*
 D3D12GraphicsAllocator.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "D3D12GraphicsAllocator.h"
#include "MoxUtils.h"
#include "D3D12PipelineState.h"
#include "D3D12CommandList.h"
#include "D3D12Device.h"
#include "D3D12UtilsInternal.h"
#include "MoxUtils.h"
#include "D3D12BufferAllocator.h"
#include "Application.h"
#include "D3D12DescHeapFactory.h"
#include "D3D12Window.h"
#include "D3D12CommandQueue.h"
#include "MoxEntity.h"
#include "MoxRenderProxy.h"

namespace Mox { 

	// Note: The non-inline definition of the constructor is necessary to forward classes used in member smart pointers!!
	// More info in this thread: https://stackoverflow.com/questions/27336779/unique-ptr-and-forward-declaration
	D3D12GraphicsAllocator::D3D12GraphicsAllocator() = default;

	D3D12GraphicsAllocator::~D3D12GraphicsAllocator()
	{
		m_DynamicBufferAllocator.reset();

		m_DescHeapFactory.reset();

	}

	Mox::D3D12Resource& D3D12GraphicsAllocator::AllocateResourceForBuffer(uint32_t InSize, Mox::RESOURCE_HEAP_TYPE InHeapType, Mox::RESOURCE_STATE InState, Mox::RESOURCE_FLAGS InFlags /*= RESOURCE_FLAGS::NONE*/)
	{
		// If passed size is 0, set it to minimum resource size
		if (InSize == 0)
			InSize = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource;
		Mox::CreateCommittedResource(
			static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner(), d3d12Resource.GetAddressOf(),
			Mox::HeapTypeToD3D12(InHeapType), InSize, Mox::ResFlagsToD3D12(InFlags), Mox::ResourceStateTypeToD3D12(InState));

		m_GraphicsResources.emplace_back( d3d12Resource );

		d3d12Resource.Reset();

		return m_GraphicsResources.back();
	}

	void D3D12GraphicsAllocator::AllocateResourceForBuffer(const Mox::BufferResourceRequest& InResourceRequest)
	{
		// TODO replace BufferResourceRequest with a single buffer reference, 
		// since we can deduce the buffer type and the size from the buffer itself.
		// 
		if (InResourceRequest.m_TargetBuffer->GetType() == BUFFER_TYPE::DYNAMIC)
		{
			Mox::BufferResource& newBufferResource = AllocateDynamicBuffer(InResourceRequest.m_TargetBuffer->GetSize());

			InResourceRequest.m_TargetBuffer->SetResource(&newBufferResource);
		}
		// TODO needs case for static buffers
	}

	Mox::BufferResource& D3D12GraphicsAllocator::AllocateDynamicBuffer(uint32_t InSize)
	{
		return m_DynamicBufferAllocator->Allocate(InSize);
	}

	Mox::Texture& D3D12GraphicsAllocator::AllocateTextureFromFile(wchar_t const* InTexturePath, Mox::TEXTURE_FILE_FORMAT InFileFormat, int32_t InMipsNum /*= 0*/, Mox::RESOURCE_FLAGS InCreationFlags /*= RESOURCE_FLAGS::NONE*/)
	{
		// TODO FIX ME
		return *m_TextureArray.back();
	}

	Mox::Texture& D3D12GraphicsAllocator::AllocateEmptyTexture(uint32_t InWidth, uint32_t InHeight, Mox::TEXTURE_TYPE InType, Mox::BUFFER_FORMAT InFormat, uint32_t InArraySize, uint32_t InMipLevels)
	{
				// TODO FIX ME
		return *m_TextureArray.back();
	}

	Mox::Resource& D3D12GraphicsAllocator::AllocateBufferCommittedResource(Mox::CommandList& InCmdList, const void* InBufferData, uint32_t InSize, Mox::RESOURCE_FLAGS InFlags /*= Mox::RESOURCE_FLAGS::NONE*/)
	{
		m_GraphicsResources.emplace_back(InCmdList, InBufferData, InSize, InFlags);
		
		return m_GraphicsResources.back();


	}


	Mox::VertexBufferView& D3D12GraphicsAllocator::AllocateVertexBufferView(Mox::VertexBuffer& InVB)
	{
		m_VertexViewArray.emplace_back( InVB ); //TODO possibly checking to not pass a certain number of allocations
		return m_VertexViewArray.back();
	}

	Mox::IndexBufferView& D3D12GraphicsAllocator::AllocateIndexBufferView(Mox::IndexBuffer& InIB, Mox::BUFFER_FORMAT InFormat)
{
		m_IndexViewArray.emplace_back( InIB, InFormat);
		return m_IndexViewArray.back();
	}

	Mox::ConstantBufferView& D3D12GraphicsAllocator::AllocateConstantBufferView(Mox::BufferResource& InResource)
	{
		// Allocate the view
		// Note: the constructor will allocate a corresponding descriptor in a CPU desc heap
		return static_cast<Mox::ConstantBufferView&>(
			m_DescHeapFactory->AddViewObject(std::make_unique<Mox::D3D12ConstantBufferView>(InResource))
			);
	}


	Mox::ShaderResourceView& D3D12GraphicsAllocator::AllocateShaderResourceView(Mox::Texture& InTexture)
	{
		// Allocate the view
		// Note: the constructor will allocate a corresponding descriptor in a CPU desc heap
		return static_cast<Mox::ShaderResourceView&>(
			m_DescHeapFactory->AddViewObject(std::make_unique<Mox::D3D12ShaderResourceView>(InTexture))
			);
	}

	Mox::ShaderResourceView& D3D12GraphicsAllocator::AllocateSrvTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, uint32_t InMostDetailedMip /*= 0*/, int32_t InMipLevels /*= -1*/, uint32_t InFirstArraySlice /*= 0*/, uint32_t InPlaneSlice /*= 0*/)
	{
		std::unique_ptr<Mox::D3D12ShaderResourceView> outSrv = std::make_unique<Mox::D3D12ShaderResourceView>(InTexture);

		outSrv->InitAsTex2DArray(InTexture, InArraySize, InMostDetailedMip, InMipLevels, InFirstArraySlice, InPlaneSlice); // TODO move the Init inside constructor

		return static_cast<Mox::ShaderResourceView&>(
			m_DescHeapFactory->AddViewObject(std::move(outSrv))
			);
	}

	Mox::UnorderedAccessView& D3D12GraphicsAllocator::AllocateUavTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, int32_t InMipSlice /*= -1*/, uint32_t InFirstArraySlice /*= 0*/, uint32_t InPlaceSlice /*= 0*/)
	{
		std::unique_ptr<Mox::D3D12UnorderedAccessView> outUav = std::make_unique<Mox::D3D12UnorderedAccessView>(InTexture, InArraySize, InMipSlice, InFirstArraySlice, InPlaceSlice);

		return static_cast<Mox::UnorderedAccessView&>(
			m_DescHeapFactory->AddViewObject(std::move(outUav))
			);
	}

	Mox::Shader& D3D12GraphicsAllocator::AllocateShader(wchar_t const* InShaderPath)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> OutFileBlob;
		Mox::ThrowIfFailed(::D3DReadFileToBlob(InShaderPath, &OutFileBlob));
		m_ShaderArray.push_back(std::make_unique<Mox::D3D12Shader>(OutFileBlob));

		return *m_ShaderArray.back();
	}

	Mox::PipelineState& D3D12GraphicsAllocator::AllocatePipelineState()
{
		m_PipelineStateArray.push_back(std::make_unique<Mox::D3D12PipelineState>());

		return *m_PipelineStateArray.back();
	}

	void D3D12GraphicsAllocator::ReserveDynamicBufferMemory(size_t InSize, void*& OutCpuPtr, D3D12_GPU_VIRTUAL_ADDRESS& OutGpuPtr)
	{

	}

	Mox::D3D12DescriptorHeap& D3D12GraphicsAllocator::GetDescriptorsCpuHeap()
	{
		return m_DescHeapFactory->GetCPUHeap();
	}

	Mox::D3D12DescriptorHeap& D3D12GraphicsAllocator::GetDescriptorsGpuHeap()
	{
		return m_DescHeapFactory->GetGPUHeap();
	}

	Mox::Window& D3D12GraphicsAllocator::AllocateWindow(Mox::WindowInitInput& InWindowInitInput)
	{
		m_WindowArray.emplace_back(std::make_unique<Mox::D3D12Window>(InWindowInitInput));

		return *m_WindowArray.back();
	}

	Mox::CommandQueue& D3D12GraphicsAllocator::AllocateCommandQueue(Device& InDevice, COMMAND_LIST_TYPE InCmdListType)
	{
		m_CommandQueueArray.emplace_back(std::make_unique<Mox::D3D12CommandQueue>(InDevice, InCmdListType));

		return *m_CommandQueueArray.back();
	}


	std::vector<Mox::RenderProxy*> D3D12GraphicsAllocator::CreateProxies(const std::vector<Mox::RenderProxyRequest>& InRequests)
	{
		std::vector<Mox::RenderProxy*> outProxies;  outProxies.reserve(InRequests.size());

		for (const Mox::RenderProxyRequest& proxyRequest : InRequests)
		{
			// Create meshes
			std::vector<Mox::Mesh*> newMeshes; newMeshes.reserve(proxyRequest.m_MeshInfos.size());
			for (const Mox::MeshCreationInfo& meshInfo : proxyRequest.m_MeshInfos)
			{
				m_MeshArray.emplace_back(std::make_unique<Mox::Mesh>(*meshInfo.m_VertexBuffer, *meshInfo.m_IndexBuffer, meshInfo.m_ShaderParameters));
				newMeshes.push_back(m_MeshArray.back().get());
			}

			// Create proxy
			m_RenderProxyArray.emplace_back(std::make_unique<Mox::RenderProxy>(newMeshes));

			Mox::RenderProxy* newProxy = m_RenderProxyArray.back().get();
			proxyRequest.m_TargetEntity->SetRenderProxy(newProxy);
			outProxies.push_back(newProxy);
		}

		return outProxies;
	}

	Mox::VertexBuffer& D3D12GraphicsAllocator::AllocateVertexBuffer(Mox::CommandList& InCmdList, const void* InData, uint32_t InStride, uint32_t InSize)
	{
		m_VertexBufferArray.emplace_back(InCmdList, InData, InStride, InSize);
		return m_VertexBufferArray.back();
	}

	Mox::IndexBuffer& D3D12GraphicsAllocator::AllocateIndexBuffer(Mox::CommandList& InCmdList, const void* InData, int32_t InSize, int32_t InElementsNum)
	{
		m_IndexBufferArray.emplace_back(InCmdList, InData, InSize, InElementsNum);
		return m_IndexBufferArray.back();
	}

	void D3D12GraphicsAllocator::OnNewFrameStarted()
	{
		// The number of partitions considered by the graphics allocator will be equal to 
		// the number of max Gpu frames in flight + the current one being computed by the renderer.
		static const uint64_t totalPartitionsNum = Application::GetMaxGpuConcurrentFramesNum() + 1;
		float currentFramePartition = (m_FrameCounter % totalPartitionsNum) / static_cast<float>(totalPartitionsNum);
		static const float fractionSize = 1.0f / totalPartitionsNum;

		m_DynamicBufferAllocator->OnFrameStarted();

		// TODO change descriptor handling the way we do with dynamic buffer allocator
		GetDescriptorsGpuHeap().SetAllowedDynamicAllocationRegion(currentFramePartition, currentFramePartition + fractionSize);

		m_FrameCounter++;
	}

	void D3D12GraphicsAllocator::OnNewFrameEnded()
	{
		m_DynamicBufferAllocator->OnFrameEnded();
	}

	void D3D12GraphicsAllocator::OnStartRenderMainView(Mox::CommandList& InCmdList)
	{


	}

	void D3D12GraphicsAllocator::Initialize()
	{
		m_DescHeapFactory = std::make_unique<Mox::D3D12DescHeapFactory>();

		// Allocate an empty resource and create the dynamic buffer allocator on it
		// Note: We are using a system similar to what described for Diligent Engine https://www.codeproject.com/Articles/1094799/Implementing-Dynamic-Resources-with-Direct-D
		Mox::D3D12Resource& dynamicBufferResource = AllocateResourceForBuffer(0, RESOURCE_HEAP_TYPE::UPLOAD, RESOURCE_STATE::GEN_READ);

		m_DynamicBufferAllocator = std::make_unique<Mox::D3D12LinearBufferAllocator>(dynamicBufferResource);

	}

	void D3D12GraphicsAllocator::StoreAndReferenceDynamicBuffer(uint32_t InRootIdx, Mox::BufferResource& InDynBuffer, Mox::ConstantBufferView& InResourceView)
	{
		// Create space for current Dynamic Buffer value
		void* cpuPtr; D3D12_GPU_VIRTUAL_ADDRESS gpuPtr;
		static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->ReserveDynamicBufferMemory(InDynBuffer.GetSize(), cpuPtr, gpuPtr);

		// Copy buffer content in the allocation
		memcpy(cpuPtr, InDynBuffer.GetData(), InDynBuffer.GetSize());

		// Reference it in the view object
		static_cast<Mox::D3D12ConstantBufferView&>(InResourceView).ReferenceBuffer(InDynBuffer);

		// Stage View's descriptor for GPU heap insertion
		Mox::D3D12ConstantBufferView& bufferView = static_cast<Mox::D3D12ConstantBufferView&>(InResourceView);

	}
}
