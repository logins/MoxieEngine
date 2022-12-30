/*
 D3D12TextureAllocator.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2022 Riccardo Loggini
*/

#include "D3D12TextureAllocator.h"
#include "MoxMath.h"
#include "D3D12Device.h"
#include "D3D12UtilsInternal.h"
#include "RangeAllocators.h"
#include "D3D12GraphicsAllocator.h"
#include "D3D12CommandList.h"

namespace Mox {

D3D12TextureAllocator::D3D12TextureAllocator(size_t InHeapSize, Mox::D3D12Resource& InStagingResource)
	: m_StagingResource(InStagingResource), m_AllocationsAlignment(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT),
	 m_PoolSize(Mox::Align(InHeapSize, m_AllocationsAlignment)), m_RangeAllocator(std::make_unique<Mox::StaticRangeAllocator>(0, m_PoolSize))
{
	// Create heap
	CD3DX12_HEAP_DESC heapDesc = CD3DX12_HEAP_DESC(InHeapSize, D3D12_HEAP_TYPE_DEFAULT);
	static_cast<Mox::D3D12Device&>(GetDevice()).GetInner()->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_OwningHeap));

}

Mox::TextureResource& D3D12TextureAllocator::Allocate(const TextureDesc& InDesc)
{
	if (!(InDesc.m_Type == Mox::TEXTURE_TYPE::TEX_2D || InDesc.m_Type == Mox::TEXTURE_TYPE::TEX_CUBE))
	{
		StopForFail("[D3D12Texture::SetGeneralTextureParams] Texture Type not handled yet.")
	}

	D3D12_RESOURCE_DESC texResDesc{
		Mox::TextureTypeToD3D12(InDesc.m_Type),
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, // Resource placement alignment is mandatory for texture resources that do not use MSAA and greater than 4KB in size
		InDesc.m_Width,
		InDesc.m_Height,
		InDesc.m_ArraySize,
		InDesc.m_MipLevelsNum,
		Mox::BufferFormatToD3D12(InDesc.m_TexelFormat),
		DXGI_SAMPLE_DESC{1,0}, // No multisample supported
		D3D12_TEXTURE_LAYOUT_UNKNOWN, // During creation, the driver chooses the most efficient layout based on other resource properties
		D3D12_RESOURCE_FLAG_NONE
	};

	// Query the expected texture size given the requested desc
	D3D12_RESOURCE_ALLOCATION_INFO allocInfo = static_cast<Mox::D3D12Device&>(GetDevice()).GetInner()->GetResourceAllocationInfo(0, 1, &texResDesc);

	// Note: The passed size will be aligned before asking for allocation.
	// This will ensure that the returned allocation offset will be aligned.
	uint32_t relativeResourceOffset = m_RangeAllocator->AllocateRange(allocInfo.SizeInBytes);

	// Create Placed Resource
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource

	Microsoft::WRL::ComPtr<ID3D12Resource> newD3D12Res;
		static_cast<Mox::D3D12Device&>(GetDevice()).GetInner()->CreatePlacedResource(
		m_OwningHeap.Get(), 
		relativeResourceOffset,
		&texResDesc,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&newD3D12Res)
		);

	Mox::Resource& newResource = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->AllocateD3D12Resource(newD3D12Res, D3D12_RES_TYPE::Texture, allocInfo.SizeInBytes);
		
	m_TextureArray.emplace_back(std::make_unique<Mox::TextureResource>(InDesc, newResource, 0, allocInfo.SizeInBytes));

	return *m_TextureArray.back().get();
}

void D3D12TextureAllocator::UpdateContent(Mox::CommandList& InCmdList, const std::vector<Mox::TextureResourceUpdate>& InTexUpdates)
{
	uint64_t intermediateOffset = 0;

	for (const Mox::TextureResourceUpdate& curUpdate : InTexUpdates)
	{
		// Expected to find a contiguous memory of subresources in the data to upload

		Mox::D3D12Resource& curTexResource = static_cast<Mox::D3D12Resource&>(curUpdate.m_TargetTexture->GetResource()->GetOwnerResource());
		// Get memory layout information (footprints) of the subresources contained in the texture resource that we want to operate on
		D3D12_RESOURCE_DESC& texResourceDesc = curTexResource.GetDesc();
		
		uint16_t subresourceUpdatesNum = curUpdate.m_UpdateDesc.size();

		// Rows num, row size and total bytes is the same for every mip 0
		std::vector <uint32_t> rowsNumVector(subresourceUpdatesNum);
		std::vector <uint64_t> rowSizeVector(subresourceUpdatesNum);

		// Note: D3D12_SUBRESOURCE_INFO holds information about the updates on CPU side we want to transfer to the target subresources on GPU!
		std::vector<D3D12_SUBRESOURCE_DATA> subresourceDataVector(subresourceUpdatesNum);

		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> mip0Footprints(subresourceUpdatesNum);

		// Layout for mip0 of every slice is the same, what changes is only the parent resource offset
		for (int i = 0; i < subresourceUpdatesNum; ++i)
		{
			const Mox::TexDataInfo& updateDesc = curUpdate.m_UpdateDesc[i];

			uint16_t mipIndexInTexture = updateDesc.m_SliceIndex * texResourceDesc.MipLevels + updateDesc.m_MipLevel;
			rowsNumVector[i] = curTexResource.GetRowsNumVector()[mipIndexInTexture];
			rowSizeVector[i] = curTexResource.GetRowSizeVector()[mipIndexInTexture];

			// Data we are sending is meant to have all the mip 0 in contiguous memory
			subresourceDataVector[i] = {
				updateDesc.m_Location,			// pData
				updateDesc.m_RowSize,			// RowPitch
				updateDesc.m_TotalSize		// DepthPitch
			};

			mip0Footprints[i] = curTexResource.GetSubresourceFootprints()[i];
		}

		UINT64 uploadEsit = ::UpdateSubresources(
			static_cast<Mox::D3D12CommandList&>(InCmdList).GetInner().Get(),		//_In_ ID3D12GraphicsCommandList * pCmdList,
			curTexResource.GetInner().Get(),										//_In_ ID3D12Resource * pDestinationResource,
			static_cast<Mox::D3D12Resource&>(m_StagingResource).GetInner().Get(),	//_In_ ID3D12Resource * pIntermediate,
			intermediateOffset,			//UINT64 IntermediateOffset,
			0,							//_In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
			subresourceUpdatesNum,		//_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
			subresourceDataVector.data()//_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA * pSrcData) noexcept
		);
			

		Check(uploadEsit > 0) // This should return the resource required size. If 0 is returned, the operation failed.
			
		intermediateOffset += Mox::Align(curUpdate.m_UpdateSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

	}

	
}

}
