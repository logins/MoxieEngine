/*
 MoxUtils.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "MoxUtils.h"
#include "D3D12UtilsInternal.h"
#include "D3D12MoxUtils.h"
#include "D3D12Device.h"
#include "MoxMath.h"
#include "D3D12CommandList.h"
#include "D3D12GraphicsAllocator.h"

#ifdef max
#undef max // This is needed to avoid conflicts with functions called max(), like chrono::milliseconds::max()
#undef min
#endif

namespace Mox
{

	ComPtr<IDXGIAdapter4> GetMainAdapter(bool InUseWarp)
	{
		// Note: warp is a virtual platform that provides backward compatibility to DX12 for older graphics devices.
		// More info at: https://docs.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp#what-is-warp 
		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;

#ifdef _DEBUG
		createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		ThrowIfFailed(::CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		if (InUseWarp)
		{
			// Retrieve the first adapter found from the factory
			ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
			// Try casting it to adapter4
			ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
		}
		else
		{
			SIZE_T maxDedicatedVideoMemory = 0;
			// Inspecting all the devices found and taking the one with largest video memory
			for (UINT i =0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc);
				if ((dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 // Avoid the adapter called the "Microsoft Basic Render Driver" adapter, which is a default adapter without display outputs
					&& SUCCEEDED(::D3D12CreateDevice(dxgiAdapter1.Get(),
						D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) // Check if current adapter can create a D3D12 device without actually creating it
					&& dxgiAdapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory
					)
				{
					maxDedicatedVideoMemory = dxgiAdapterDesc.DedicatedVideoMemory;
					ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
				}
			}
		}
		return dxgiAdapter4;
	}

	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> InAdapter)
	{
		ComPtr<ID3D12Device2> d3dDevice2;
		ThrowIfFailed(::D3D12CreateDevice(InAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3dDevice2)));
#if _DEBUG // Adding Info Queue filters
		ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(d3dDevice2.As(&infoQueue)))
		{
			// When a message with the following severities passes trough the Storage Filter, the program will break.
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		}
		else
		{
			assert("We were not able to get the InfoQueue from device... Maybe the Debug Layer is disabled..");
		}
		// Following message severities will be suppressed
		D3D12_MESSAGE_SEVERITY suppressedSeverities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		// Following messages will be suppressed based on their id
		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
		};

		D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
		infoQueueFilter.DenyList.NumSeverities = _countof(suppressedSeverities);
		infoQueueFilter.DenyList.pSeverityList = suppressedSeverities;
		infoQueueFilter.DenyList.NumIDs = _countof(denyIds);
		infoQueueFilter.DenyList.pIDList = denyIds;
		// Note: we can also deny entire message categories
		ThrowIfFailed(infoQueue->PushStorageFilter(&infoQueueFilter));
#endif
		return d3dDevice2;
	}

	ComPtr<ID3D12CommandQueue> CreateCommandQueue(ComPtr<ID3D12Device2> InDevice, D3D12_COMMAND_LIST_TYPE InType)
	{
		ComPtr<ID3D12CommandQueue> d3d12CmdQueue;

		D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
		cmdQueueDesc.Type = InType;
		cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		cmdQueueDesc.NodeMask = 0; // Assuming using 1 GPU
		cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		ThrowIfFailed(InDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&d3d12CmdQueue)));

		return d3d12CmdQueue;
	}

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> InDevice, D3D12_DESCRIPTOR_HEAP_TYPE InType, UINT InNumDescriptors)
	{
		ComPtr<ID3D12DescriptorHeap> descrHeap;

		D3D12_DESCRIPTOR_HEAP_DESC descrHeapDesc = {};
		descrHeapDesc.Type = InType;
		descrHeapDesc.NumDescriptors = InNumDescriptors;
		descrHeapDesc.NodeMask = 0;
		descrHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ThrowIfFailed(InDevice->CreateDescriptorHeap(&descrHeapDesc, IID_PPV_ARGS(&descrHeap)));

		return descrHeap;
	}

	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(ComPtr<ID3D12Device2> InDevice, D3D12_COMMAND_LIST_TYPE InCmdListType)
	{
		ComPtr<ID3D12CommandAllocator> cmdAllocator;
		ThrowIfFailed(InDevice->CreateCommandAllocator(InCmdListType, IID_PPV_ARGS(&cmdAllocator)));
		return cmdAllocator;
	}

	ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(ComPtr<ID3D12Device2> InDevice, ComPtr<ID3D12CommandAllocator> InCmdAllocator, D3D12_COMMAND_LIST_TYPE InCmdListType, bool InInitClosed /*= true*/)
	{
		ComPtr<ID3D12GraphicsCommandList2> cmdList;
		ThrowIfFailed(InDevice->CreateCommandList(0, InCmdListType, InCmdAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)));
		// Note: PSO parameter is optional and we can initialize the command list with it
		// Note: the initial state of a newly created command list is Open, so we manually need to close it.
		if(InInitClosed)
			ThrowIfFailed(cmdList->Close()); // Close it because the beginning of the render method will execute reset on the cmdList
	
		return cmdList;
	}	

	ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device2> InDevice)
	{
		ComPtr<ID3D12Fence> fence;
		ThrowIfFailed(InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		return fence;
	}

	HANDLE CreateFenceEventHandle()
	{
		HANDLE eventHandle;
		eventHandle = ::CreateEvent(
			NULL,	// Handle cannot be inherited by child processes
			FALSE,	// This is going to be an auto-reset event object, after being signaled, it will automatically return in non-signaled state
			FALSE,	// Initial event state is non-signaled 
			NULL);	// Name of the event object, for events comparison
		assert(eventHandle && "[Mox] Failed to create fence event.");

		return eventHandle;
	}

	void WaitForFenceValue(ComPtr<ID3D12Fence> InFence, uint64_t InFenceValue, HANDLE InFenceEvent, std::chrono::milliseconds InMaxDuration /*= std::chrono::milliseconds::max()*/)
	{
		if (InFence->GetCompletedValue() < InFenceValue)
		{
			ThrowIfFailed(InFence->SetEventOnCompletion(InFenceValue, InFenceEvent));

			::WaitForSingleObject(InFenceEvent, static_cast<DWORD>(InMaxDuration.count())); // count() returns the number of ticks
		}
	}

	void ClearRTV(ID3D12GraphicsCommandList2* InCmdList, D3D12_CPU_DESCRIPTOR_HANDLE InRTVCPUDescHandle, FLOAT* InClearColor)
	{
		InCmdList->ClearRenderTargetView(InRTVCPUDescHandle, InClearColor, 0, nullptr);
	}

	void ClearDepth(ID3D12GraphicsCommandList2* InCmdList, D3D12_CPU_DESCRIPTOR_HANDLE InDepthCPUDescHandle, FLOAT InDepth /*= 1.0f*/)
	{
		InCmdList->ClearDepthStencilView(InDepthCPUDescHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void UpdateBufferResource(ComPtr<ID3D12Device2> InDevice, ComPtr<ID3D12GraphicsCommandList2> InCmdList, ID3D12Resource** InDestResource, ID3D12Resource** InIntermediateResource, size_t InNunElements, size_t InElementSize, const void* InBufferData, D3D12_RESOURCE_FLAGS InFlags)
	{
		// Note: ID3D12Resource** InDestResource, ID3D12Resource** InIntermediateResource are CPU Buffer Data !!!
		// We create them on CPU, then we use them to update the corresponding SubResouce on the GPU!
		size_t bufferSize = InNunElements * InElementSize;
		// Create a committed resource for the GPU resource in a default heap
		// Note: CreateCommittedResource will allocate a resource heap and a resource in it in GPU memory, then it will return the corresponding GPUVirtualAddress that will be stored inside the ID3D12Resource object,
		// so that we can reference that GPU memory address from CPU side.
		CreateCommittedResource(InDevice, InDestResource, D3D12_HEAP_TYPE_DEFAULT, bufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST);
		if (InBufferData)
		{ // Create a committed resource in an upload heap to upload content to the first resource
			CreateCommittedResource(InDevice, InIntermediateResource, D3D12_HEAP_TYPE_UPLOAD, bufferSize, InFlags, D3D12_RESOURCE_STATE_GENERIC_READ);

			// Now that both copy and dest resource are created on CPU, we can use them to update the corresponding GPU SubResource
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = InBufferData; // Pointer to the memory block that contains the subresource data on CPU
			subresourceData.RowPitch = bufferSize; // Physical size in Bytes of the subresource data
			subresourceData.SlicePitch = subresourceData.RowPitch; // Size of each slice for the resource, since we assume only 1 slice, this corresponds to the size of the entire resource
			::UpdateSubresources(InCmdList.Get(), *InDestResource, *InIntermediateResource, 0, 0, 1, &subresourceData);
		}
	}

	void CreateCommittedResource(ComPtr<ID3D12Device2> InDevice, ID3D12Resource** InResource, D3D12_HEAP_TYPE InHeapType, uint64_t InBufferSize, D3D12_RESOURCE_FLAGS InFlags, D3D12_RESOURCE_STATES InInitialStates)
	{
		ThrowIfFailed(InDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(InHeapType),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(InBufferSize, InFlags),
			InInitialStates,
			nullptr, IID_PPV_ARGS(InResource)
		));
	}

	void CreateDepthStencilCommittedResource(ComPtr<ID3D12Device2> InDevice, ID3D12Resource** InResource, uint64_t InWidth, uint64_t InHeight, D3D12_RESOURCE_STATES InInitialStates, D3D12_CLEAR_VALUE* InClearValue)
	{
		ThrowIfFailed(InDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, InWidth, InHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			InInitialStates, InClearValue, IID_PPV_ARGS(InResource)
		));
	}

	void CreateDepthStencilView(ComPtr<ID3D12Device2> InDevice, ID3D12Resource* InResource, D3D12_CPU_DESCRIPTOR_HANDLE& InDSVCPUDescHandle)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D = { 0 };
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		InDevice->CreateDepthStencilView(InResource, &dsvDesc, InDSVCPUDescHandle);
	}

	void SignalCmdQueue(ComPtr<ID3D12CommandQueue> InCmdQueue, ComPtr<ID3D12Fence> InFence, uint64_t& OutFenceValue)
	{
		++OutFenceValue;

		ThrowIfFailed(InCmdQueue->Signal(InFence.Get(), OutFenceValue));
	}

	void FlushCmdQueue(ComPtr<ID3D12CommandQueue> InCmdQueue, ComPtr<ID3D12Fence> InFence, HANDLE InFenceEvent, uint64_t& OutFenceValue)
	{
		SignalCmdQueue(InCmdQueue, InFence, OutFenceValue);

		WaitForFenceValue(InFence, OutFenceValue, InFenceEvent);
	}

	// Note: enabling debug layer after creating a device will cause the runtime to remove the device.
	// Always enable debug layer before the device to be created.
	void EnableDebugLayer_Internal()
	{
#ifdef _DEBUG
		ComPtr<ID3D12Debug> debugInterface;
		ThrowIfFailed(::D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));

		debugInterface->EnableDebugLayer();
#endif
	}

	void ReadFileToBlob(LPCWSTR InFilePath, ID3DBlob** OutFileBlob)
	{
		return ThrowIfFailed(::D3DReadFileToBlob(InFilePath, OutFileBlob));
	}

	ComPtr<ID3D12RootSignature> SerializeAndCreateRootSignature(ComPtr<ID3D12Device2> InDevice, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC* InRootSigDesc, D3D_ROOT_SIGNATURE_VERSION InVersion)
	{
		// Create Root Signature Blob
		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		ThrowIfFailed(::D3DX12SerializeVersionedRootSignature(InRootSigDesc, InVersion, 
			rootSignatureBlob.GetAddressOf(), errorBlob.GetAddressOf()));

		// Create the Root Signature object from the blob
		ComPtr<ID3D12RootSignature> rootSignature;
		ThrowIfFailed(InDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

		return rootSignature;
	}

	bool LoadDDSTextureData(const char* InFilePath, 
		TextureDesc& OutTexDesc, void*& OutLoadedDataPtr, size_t& OutLoadedDataSize, std::vector<TexDataInfo>& OutSubresInfo)
	{
		// TODO Validate file is present and width .dds extension
		/*
		if (texture)
		{
			*texture = nullptr;
		}
		if (alphaMode)
		{
			*alphaMode = DDS_ALPHA_MODE_UNKNOWN;
		}
		if (isCubeMap)
		{
			*isCubeMap = false;
		}

		if (!d3dDevice || !fileName || !texture)
		{
			return E_INVALIDARG;
		}

		const DDS_HEADER* header = nullptr;
		const uint8_t* bitData = nullptr;
		size_t bitSize = 0;

		HRESULT hr = LoadTextureDataFromFile(fileName,
			ddsData,
			&header,
			&bitData,
			&bitSize
		);
		if (FAILED(hr))
		{
			return hr;
		}
*/
		return false;
	}

	void D3D12VertexBufferView::ReferenceResource(Mox::BufferResource& InVB)
	{
		// Note: A Vertex Buffer View does not use any descriptor from a desc heap
		m_VertexBufferView.BufferLocation = InVB.GetGpuPtr();
		m_VertexBufferView.SizeInBytes = InVB.GetSize();
		m_VertexBufferView.StrideInBytes = InVB.GetStride();
	}

	void D3D12IndexBufferView::ReferenceResource(Mox::BufferResource& InIB, Mox::BUFFER_FORMAT InFormat, uint32_t InElementsNum)
	{
		m_ElementsNum = InElementsNum; // TODO elements num can be deduced by 
		// providing a function BufferFormatSize(InFormat) to be used as ElementsNum = bufferSize / bufferFormat

		// Note: An Index Buffer View does not use any descriptor from a desc heap
		m_IndexBufferView.BufferLocation = InIB.GetGpuPtr();
		m_IndexBufferView.SizeInBytes = InIB.GetSize();
		m_IndexBufferView.Format = Mox::BufferFormatToD3D12(InFormat);
	}

	D3D12ShaderResourceView::D3D12ShaderResourceView(Mox::TextureResource& InTextureToReference)
		: ShaderResourceView(InTextureToReference.GetOwnerResource())
	{
		InitAsTex2DOrCubemap(InTextureToReference);
	}

	void D3D12ShaderResourceView::InitAsTex2DOrCubemap(Mox::TextureResource& InTexture)
{
		if (!m_CpuAllocatedRange)
		{
			// Allocate descriptor in CPU descriptor heap
			m_CpuAllocatedRange = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsCpuHeap().AllocateStaticRange(1);
		}
		// TODO Note: we are currently assuming we only handle a single descriptor and never a range... also in D3D12ConstantBufferView::ReferenceBuffer

		// Generate View Desc
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = Mox::BufferFormatToD3D12(InTexture.GetFormat());
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (InTexture.GetType() == Mox::TEXTURE_TYPE::TEX_CUBE)
		{
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.TextureCube.MipLevels = InTexture.GetMipLevelsNum();
		}
		else
		{
			StopForFail("SRV referenced texture type not handled yet!")
		}

		// Instantiate View
		Mox::D3D12Device& d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice());

		d3d12Device.GetInner()->CreateShaderResourceView(static_cast<Mox::D3D12Resource&>(InTexture.GetOwnerResource()).GetInner().Get(), &viewDesc, m_CpuAllocatedRange->m_FirstCpuHandle);

	}

	bool D3D12ShaderResourceView::IsGpuAllocated()
	{
		return false; // TODO fix me
	}

	void D3D12ShaderResourceView::InitAsTex2DArray(Mox::TextureResource& InTexture, uint32_t InArraySize, uint32_t InMostDetailedMip, uint32_t InMipLevels, uint32_t InFirstArraySlice, uint32_t InPlaceSlice)
	{
		// Allocate static descriptor in the CPU-only desc heap
		if (!m_CpuAllocatedRange)
		{
			m_CpuAllocatedRange = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsCpuHeap().AllocateStaticRange(1);
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Format = Mox::BufferFormatToD3D12(InTexture.GetFormat());;
		srvDesc.Texture2DArray.ArraySize = InArraySize;
		srvDesc.Texture2DArray.MostDetailedMip = InMostDetailedMip;
		srvDesc.Texture2DArray.FirstArraySlice = InFirstArraySlice;
		srvDesc.Texture2DArray.MipLevels = InMipLevels;
		srvDesc.Texture2DArray.PlaneSlice = InPlaceSlice;

		// Instantiate View
		Mox::D3D12Device& d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice());

		d3d12Device.GetInner()->CreateShaderResourceView(static_cast<Mox::D3D12Resource&>(InTexture.GetOwnerResource()).GetInner().Get(), &srvDesc, m_CpuAllocatedRange->m_FirstCpuHandle);
	}

	D3D12UnorderedAccessView::D3D12UnorderedAccessView(Mox::TextureResource& InTexture, uint32_t InArraySize, uint32_t InMipSlice, uint32_t InFirstArraySlice, uint32_t InPlaneSlice)
		: UnorderedAccessView(InTexture.GetOwnerResource())
	{
		// Allocate static descriptor in the CPU-only desc heap
		if (!m_CpuAllocatedRange)
		{
			m_CpuAllocatedRange = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsCpuHeap().AllocateStaticRange(1);
		}

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Format = Mox::BufferFormatToD3D12(InTexture.GetFormat());;
		uavDesc.Texture2DArray.ArraySize = InArraySize;
		uavDesc.Texture2DArray.FirstArraySlice = InFirstArraySlice;
		uavDesc.Texture2DArray.MipSlice = InMipSlice;
		uavDesc.Texture2DArray.PlaneSlice = InPlaneSlice;

		// Instantiate View
		Mox::D3D12Device& d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice());

		d3d12Device.GetInner()->CreateUnorderedAccessView(static_cast<Mox::D3D12Resource&>(InTexture.GetOwnerResource()).GetInner().Get(), nullptr, &uavDesc, m_CpuAllocatedRange->m_FirstCpuHandle);
	}

	bool D3D12UnorderedAccessView::IsGpuAllocated()
	{
		return false; // TODO fix me
	}

	D3D12ConstantBufferView::D3D12ConstantBufferView(Mox::BufferResource& InBuffer)
		: ConstantBufferView(InBuffer)
	{
		ReferenceBuffer(InBuffer);
	}

	void D3D12ConstantBufferView::ReferenceBuffer(Mox::BufferResource& InBuffer)
	{
		if (!m_CpuAllocatedRange)
		{
			// Allocate descriptor in CPU descriptor heap

			m_CpuAllocatedRange = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsCpuHeap().AllocateStaticRange(1); // Even if we have a dynamic buffer, the descriptor on the CPU staging desc heap will be allocated statically, and change value frequently
		}

		// Generate View Desc
		D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
		viewDesc.BufferLocation = static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(InBuffer.GetGpuPtr());
		viewDesc.SizeInBytes = InBuffer.GetSize();

		// Instantiate View
		Mox::D3D12Device& d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice());

		d3d12Device.GetInner()->CreateConstantBufferView(&viewDesc, m_CpuAllocatedRange->m_FirstCpuHandle);
	}

	void D3D12ConstantBufferView::RebuildResourceReference()
	{
		ReferenceBuffer(m_ReferencedBuffer);
	}


	D3D12Resource::D3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> InD3D12Res, D3D12_RES_TYPE InResType, size_t InSize) 
		: m_D3D12Resource(InD3D12Res),  m_Desc(InD3D12Res->GetDesc())
	{
		
		m_DataSize = InSize;

		if (InResType == D3D12_RES_TYPE::Buffer)
		{
			m_GpuPtr = m_D3D12Resource->GetGPUVirtualAddress();
			m_Alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
		}
		else if (InResType == D3D12_RES_TYPE::Texture)
		{
			m_GpuPtr = 0; // Texture resources do not hold a GPU virtual address
			m_Alignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
			m_SubresourcesNum = m_Desc.MipLevels * m_Desc.DepthOrArraySize;

			// Filling subresources footprint data
			m_SubresourceFootprints.resize(m_SubresourcesNum);
			m_RowsNumVector.resize(m_SubresourcesNum);
			m_RowSizeVector.resize(m_SubresourcesNum);
			m_TotalBytesVector.resize(m_SubresourcesNum);


			static_cast<Mox::D3D12Device&>(GetDevice()).GetInner()->GetCopyableFootprints(
				&m_Desc,
				0,
				GetSubresourcesNum(),
				0,
				m_SubresourceFootprints.data(),
				GetRowsNumVector().data(),
				m_RowSizeVector.data(),
				m_TotalBytesVector.data()
			);

		}

	}

	D3D12Resource::D3D12Resource(Mox::D3D12_RES_TYPE InResType, Mox::RESOURCE_HEAP_TYPE InHeapType, size_t InSize, 
		Mox::RESOURCE_FLAGS InFlags, Mox::RESOURCE_STATE InState)
	{
		m_Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		Check(InSize > 0)

		m_DataSize = Mox::Align(InSize, m_Alignment);

		Mox::CreateCommittedResource(static_cast<Mox::D3D12Device&>(
			Mox::GetDevice()).GetInner(),
			m_D3D12Resource.GetAddressOf(),
			HeapTypeToD3D12(InHeapType),
			m_DataSize, Mox::ResFlagsToD3D12(InFlags),
			Mox::ResStateTypeToD3D12(InState));

		m_GpuPtr = m_D3D12Resource->GetGPUVirtualAddress();
		m_Desc = m_D3D12Resource->GetDesc();
	}



}

