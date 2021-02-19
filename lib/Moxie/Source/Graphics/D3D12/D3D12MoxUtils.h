/*
 MoxUtils.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef D3D12MoxUtils_h__
#define D3D12MoxUtils_h__

#include <iostream>
#include <chrono>
#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include "GraphicsTypes.h"
#include "D3D12DescHeapFactory.h"

#ifdef max
#undef max // This is needed to avoid conflicts with functions called max(), like chrono::milliseconds::max()
#endif

namespace Mox {

	using namespace Microsoft::WRL;

	void PrintHello();

	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetMainAdapter(bool InUseWarp);

	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> InAdapter);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CreateCommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, D3D12_COMMAND_LIST_TYPE InType);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, D3D12_DESCRIPTOR_HEAP_TYPE InType, UINT InNumDescriptors);

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, D3D12_COMMAND_LIST_TYPE InCmdListType);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> InCmdAllocator, D3D12_COMMAND_LIST_TYPE InCmdListType, bool InInitClosed = true);

	Microsoft::WRL::ComPtr<ID3D12Fence> CreateFence(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice);

	HANDLE CreateFenceEventHandle();

	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> InCmdList, Microsoft::WRL::ComPtr<ID3D12Resource> InResource, D3D12_RESOURCE_STATES InBeforeStates, D3D12_RESOURCE_STATES InAfterStates);

	void ClearRTV(ID3D12GraphicsCommandList2* InCmdList, D3D12_CPU_DESCRIPTOR_HANDLE InRTVCPUDescHandle, FLOAT* InClearColor);

	void ClearDepth(ID3D12GraphicsCommandList2* InCmdList, D3D12_CPU_DESCRIPTOR_HANDLE InDepthCPUDescHandle, FLOAT InDepth = 1.0f);

	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> InCmdList, ID3D12Resource** InDestResource, ID3D12Resource** InIntermediateResource,
		size_t InNunElements, size_t InElementSize, const void* InBufferData, D3D12_RESOURCE_FLAGS InFlags = D3D12_RESOURCE_FLAG_NONE
		);

	void CreateCommittedResource(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, ID3D12Resource** InResource, D3D12_HEAP_TYPE InHeapType, uint64_t InBufferSize, D3D12_RESOURCE_FLAGS InFlags, D3D12_RESOURCE_STATES InInitialStates);

	void CreateDepthStencilCommittedResource(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, ID3D12Resource** InResource, uint64_t InWidth, uint64_t InHeight, D3D12_RESOURCE_STATES InInitialStates, D3D12_CLEAR_VALUE* InClearValue);

	void CreateDepthStencilView(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, ID3D12Resource* InResource, D3D12_CPU_DESCRIPTOR_HANDLE& InDSVCPUDescHandle);

	void SignalCmdQueue(Microsoft::WRL::ComPtr<ID3D12CommandQueue> InCmdQueue, Microsoft::WRL::ComPtr<ID3D12Fence> InFence, uint64_t& OutFenceValue);

	// Stalls the thread up until the InFenceEvent is signaled with InFenceValue, or when optional InMaxDuration has passed
	void WaitForFenceValue(Microsoft::WRL::ComPtr<ID3D12Fence> InFence, uint64_t InFenceValue, HANDLE InFenceEvent, std::chrono::milliseconds InMaxDuration = std::chrono::milliseconds::max());

	void FlushCmdQueue(Microsoft::WRL::ComPtr<ID3D12CommandQueue> InCmdQueue, Microsoft::WRL::ComPtr<ID3D12Fence> InFence, HANDLE InFenceEvent, uint64_t& OutFenceValue);

	void EnableDebugLayer_Internal();

	void ReadFileToBlob(LPCWSTR InFilePath, ID3DBlob** OutFileBlob);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> SerializeAndCreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device2> InDevice, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC* InRootSigDesc, D3D_ROOT_SIGNATURE_VERSION InVersion);

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}

	struct D3D12CpuDescriptorHandle : public Mox::CpuDescHandle{
		D3D12CpuDescriptorHandle() {}
		D3D12CpuDescriptorHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE InDescHandle)
			: m_DescHandle(InDescHandle)
		{
				IsBound = true;
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE& GetInner() { return m_DescHandle; }
	private:
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_DescHandle;
	};

	struct D3D12_GPU_DESC_HANDLE: public Mox::GPU_DESC_HANDLE {
		D3D12_GPU_DESC_HANDLE() {}
		D3D12_GPU_DESC_HANDLE(CD3DX12_GPU_DESCRIPTOR_HANDLE InDescHandle)
			: m_DescHandle(InDescHandle)
		{
			IsBound = true;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE& GetInner() { return m_DescHandle; }
	private:
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_DescHandle;
	};

	struct D3D12Resource : public Mox::Buffer {
		D3D12Resource() : m_D3D12Resource(nullptr) {}; // Allow empty D3D12Resource for cases like DummyBuffer
		D3D12Resource& operator=(const D3D12Resource& InObjToCopy) = default;
		D3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> InResource)
			: m_D3D12Resource(InResource)
		{
			if (InResource)
			{
				auto resourceDesc = m_D3D12Resource->GetDesc();
				m_AlignmentSize = resourceDesc.Alignment;
				m_DataSize = resourceDesc.Width * resourceDesc.Height;
			}
		}
		Microsoft::WRL::ComPtr<ID3D12Resource>& GetInner() { return m_D3D12Resource; }
		void SetInner(Microsoft::WRL::ComPtr<ID3D12Resource> InResource) { m_D3D12Resource = InResource; }
		uint64_t GetSizeInBytes() const { return m_DataSize; }
		void Map(void** OutCpuPp) { m_D3D12Resource->Map(0, nullptr, OutCpuPp); }
		void UnMap() { m_D3D12Resource->Unmap(0, nullptr); }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_D3D12Resource;

	};

	struct D3D12DynamicBuffer : public Mox::DynamicBuffer {

		virtual void SetData(void* InData, size_t InSize, size_t InAlignmentSize) override;

	};

	struct D3D12Texture : public Mox::Texture {
		D3D12Texture() = default; // Allow empty D3D12Texture for cases like DummyTexture

		D3D12Texture(uint32_t InWidth, uint32_t InHeight, Mox::TEXTURE_TYPE InType, Mox::BUFFER_FORMAT InFormat, uint32_t InArraySize, uint32_t InMipLevels);

		// Constructor to load the texture from file. It will not upload it to GPU so that has to be done manually after creating the texture.
		D3D12Texture(const wchar_t* InResourcePath, Mox::TEXTURE_FILE_FORMAT InFileFormat, int32_t InMipsNum, Mox::RESOURCE_FLAGS InCreationFlags);

		virtual void UploadToGPU(Mox::CommandList& InCommandList, Mox::Buffer& InIntermediateBuffer) override;

		virtual void InstantiateOnGPU() override;

		virtual size_t GetGPUSize() override;

		Microsoft::WRL::ComPtr<ID3D12Resource>& GetInner() { return m_D3D12Resource; }
	private:

		void SetGeneralTextureParams(uint32_t InWidth, uint32_t InHeight, Mox::TEXTURE_TYPE InType, Mox::BUFFER_FORMAT InFormat, uint32_t InArraySize, uint32_t InMipLevels, Mox::RESOURCE_FLAGS InCreationFlags);
		CD3DX12_RESOURCE_DESC m_TextureDesc;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_D3D12Resource;
		std::vector<D3D12_SUBRESOURCE_DATA> m_SubresourceDesc;

	};

	struct D3D12ConstantBufferView : public Mox::ConstantBufferView
	{
		D3D12ConstantBufferView() = default;

		D3D12ConstantBufferView(Mox::Buffer& InResource);
				
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescHandle() { return m_CpuAllocatedRange->m_FirstCpuHandle; }

		uint32_t GetRangeSize() { return m_CpuAllocatedRange->m_RangeSize; }

		void ReferenceBuffer(D3D12_GPU_VIRTUAL_ADDRESS InBufferGPUAddress, size_t InBufferSize);

		virtual void ReferenceBuffer(Mox::Buffer& InResource, size_t InDataSize, size_t InStrideSize) override;

		// Descriptor range referenced by this View object.
		// Note: The StaticDescAllocation destructor will declared the relative descriptors to be stale and they will be cleared at the end of the frame
		std::unique_ptr<Mox::StaticDescAllocation> m_CpuAllocatedRange;
		
		std::unique_ptr<Mox::StaticDescAllocation> m_GpuAllocatedRange;
	};

	struct D3D12ShaderResourceView : public Mox::ShaderResourceView
	{
		D3D12ShaderResourceView() = default;

		D3D12ShaderResourceView(Mox::Texture& InTextureToReference);

		void InitAsTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, uint32_t InMostDetailedMip, uint32_t InMipLevels, uint32_t InFirstArraySlice, uint32_t InPlaceSlice);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescHandle() { return m_CpuAllocatedRange->m_FirstCpuHandle; }

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescHandle() { return m_GpuAllocatedRange->m_FirstGpuHandle; }

		virtual void InitAsTex2DOrCubemap(Mox::Texture& InTexture);

		// Descriptor range referenced by this View object.
		// Note: The Allocated Desc Range destructor will declared the relative descriptors to be stale and they will be cleared at the end of the frame
		// Refers to a CPU desc heap that is used to stage descriptors
		std::unique_ptr<Mox::StaticDescAllocation> m_CpuAllocatedRange;
		// Refers to the shader visible descriptor in the desc heap used by command lists
		std::unique_ptr<Mox::StaticDescAllocation> m_GpuAllocatedRange;
	};

	struct D3D12UnorderedAccessView : public Mox::UnorderedAccessView
	{
		D3D12UnorderedAccessView() = default;

		D3D12UnorderedAccessView(Mox::Texture& InTextureToReference);

		void InitAsTex2DArray(Mox::Texture& InTexture, uint32_t InArraySize, uint32_t InMipSlice, uint32_t InFirstArraySlice, uint32_t InPlaneSlice);

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescHandle() { return m_CpuAllocatedRange->m_FirstCpuHandle; }

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescHandle() { return m_GpuAllocatedRange->m_FirstGpuHandle; }


		// Descriptor range referenced by this View object.
		// Note: The Allocated Desc Range destructor will declared the relative descriptors to be stale and they will be cleared at the end of the frame
		// Refers to a CPU desc heap that is used to stage descriptors
		std::unique_ptr<Mox::StaticDescAllocation> m_CpuAllocatedRange;
		// Refers to the shader visible descriptor in the desc heap used by command lists
		std::unique_ptr<Mox::StaticDescAllocation> m_GpuAllocatedRange;
	};

	struct D3D12VertexBufferView : public Mox::VertexBufferView {
		virtual void ReferenceResource(Mox::Resource& InResource, size_t DataSize, size_t StrideSize);
		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	};

	struct D3D12IndexBufferView : public Mox::IndexBufferView {
		virtual void ReferenceResource(Mox::Resource& InResource, size_t InDataSize, Mox::BUFFER_FORMAT InFormat);
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};

	
	struct D3D12Rect : public Mox::Rect {
		D3D12Rect(int32_t InLeft, int32_t InTop, int32_t InRight, int32_t InBottom)
			: D3d12Rect(InLeft, InTop, InRight, InBottom)
		{ }
		CD3DX12_RECT D3d12Rect;
	};

	struct D3D12ViewPort : public Mox::ViewPort {
		D3D12ViewPort(float InTopLeftX, float InTopLeftY, float InWidth, float InHeight)
			: D3d12Viewport(InTopLeftX, InTopLeftY, InWidth, InHeight)
		{ }
		CD3DX12_VIEWPORT D3d12Viewport;

		virtual void SetWidthAndHeight(float InWidth, float InHeight) override {
			D3d12Viewport.Width = InWidth; D3d12Viewport.Height = InHeight;
		}

	};

	struct D3D12Shader : public Mox::Shader {
		D3D12Shader(Microsoft::WRL::ComPtr<ID3DBlob> InShaderBlob) : m_ShaderBlob(InShaderBlob) { }
		Microsoft::WRL::ComPtr<ID3DBlob> m_ShaderBlob;
	};

}

#endif // D3D12MoxUtils_h__
