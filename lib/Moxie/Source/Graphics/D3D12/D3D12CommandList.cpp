/*
 D3D12CommandList.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "D3D12CommandList.h"
#include "GraphicsTypes.h"
#include "d3dx12.h"
#include "MoxUtils.h"
#include "D3D12UtilsInternal.h"
#include "D3D12Device.h"
#include "D3D12PipelineState.h"
#include "D3D12Window.h"
#include "MoxUtils.h"
#include "D3D12GraphicsAllocator.h"

namespace Mox { 

	D3D12CommandList::D3D12CommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> InCmdList, Mox::Device& InOwningDevice) 
		: CommandList(InOwningDevice), m_D3D12CmdList(InCmdList)
	{
	}

	void D3D12CommandList::ResourceBarriers(const TransitionInfoVector& InTransitions)
	{
		std::vector<D3D12_RESOURCE_BARRIER> transitionBarriers; transitionBarriers.reserve(InTransitions.size());
		Mox::Resource* targetResource; Mox::RESOURCE_STATE beforeState; Mox::RESOURCE_STATE afterState;
		for (const auto& curTransition : InTransitions)
		{
			auto [targetResource, beforeState, afterState] = curTransition;
			transitionBarriers.emplace_back(
				CD3DX12_RESOURCE_BARRIER::Transition(
					static_cast<Mox::D3D12Resource*>(targetResource)->GetInner().Get(),
					Mox::ResStateTypeToD3D12(beforeState), Mox::ResStateTypeToD3D12(afterState))
			);
		}

		m_D3D12CmdList->ResourceBarrier(transitionBarriers.size(), static_cast<D3D12_RESOURCE_BARRIER*>(transitionBarriers.data()));
	}

	void D3D12CommandList::ClearRTV(Mox::CpuDescHandle& InDescHandle, float* InColor)
	{
		m_D3D12CmdList->ClearRenderTargetView(static_cast<Mox::D3D12CpuDescriptorHandle&>(InDescHandle).GetInner(), InColor, 0, nullptr);
	}

	void D3D12CommandList::ClearDepth(Mox::CpuDescHandle& InDescHandle)
	{
		m_D3D12CmdList->ClearDepthStencilView(static_cast<Mox::D3D12CpuDescriptorHandle&>(InDescHandle).GetInner(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	}

	void D3D12CommandList::Close()
	{
		m_D3D12CmdList->Close();
	}

	void D3D12CommandList::SetPipelineStateAndResourceBinder(Mox::PipelineState& InPipelineState)
	{
		Mox::D3D12PipelineState& d3d12PSO = static_cast<Mox::D3D12PipelineState&>(InPipelineState);

		// Set PSO
		m_D3D12CmdList->SetPipelineState(d3d12PSO.GetInnerPSO().Get());

		// Set root signature
		if(InPipelineState.IsGraphics())
			m_D3D12CmdList->SetGraphicsRootSignature(d3d12PSO.GetInnerRootSignature().Get());
		else
			m_D3D12CmdList->SetComputeRootSignature(d3d12PSO.GetInnerRootSignature().Get());

		// Bind descriptor heap(s)
		m_D3D12CmdList->SetDescriptorHeaps(1, static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsGpuHeap().GetInner().GetAddressOf());

	}

	void D3D12CommandList::SetInputAssemblerData(Mox::PRIMITIVE_TOPOLOGY InPrimTopology, Mox::VertexBufferView& InVertexBufView, Mox::IndexBufferView& InIndexBufView)
	{
		m_D3D12CmdList->IASetPrimitiveTopology(Mox::PrimitiveTopoToD3D12(InPrimTopology));
		m_D3D12CmdList->IASetVertexBuffers(0, 1, &static_cast<Mox::D3D12VertexBufferView&>(InVertexBufView).m_VertexBufferView);
		m_D3D12CmdList->IASetIndexBuffer(&static_cast<Mox::D3D12IndexBufferView&>(InIndexBufView).m_IndexBufferView);
	}

	void D3D12CommandList::SetViewportAndScissorRect(Mox::ViewPort& InViewport, Mox::Rect& InScissorRect)
	{
		// Creating a viewport and scissor rect on the fly
		m_D3D12CmdList->RSSetViewports(1, &static_cast<Mox::D3D12ViewPort&>(InViewport).D3d12Viewport);
		m_D3D12CmdList->RSSetScissorRects(1, &static_cast<Mox::D3D12Rect&>(InScissorRect).D3d12Rect);
	}

	void D3D12CommandList::SetRenderTargetFromWindow(Mox::Window& InWindow)
	{
		Mox::D3D12Window& d3d12Window = static_cast<Mox::D3D12Window&>(InWindow);
		m_D3D12CmdList->OMSetRenderTargets(1, &d3d12Window.GetCurrentRTVDescHandle(), FALSE, &d3d12Window.GetCuttentDSVDescHandle());
	}

	void D3D12CommandList::SetGraphicsRootConstants(uint64_t InRootParameterIndex, uint64_t InNum32BitValuesToSet, const void* InSrcData, uint64_t InDestOffsetIn32BitValues)
	{
		m_D3D12CmdList->SetGraphicsRoot32BitConstants(InRootParameterIndex, InNum32BitValuesToSet, InSrcData, InDestOffsetIn32BitValues);
	}

	void D3D12CommandList::SetComputeRootConstants(uint64_t InRootParameterIndex, uint64_t InNum32BitValuesToSet, const void* InSrcData, uint64_t InDestOffsetIn32BitValues)
	{
		m_D3D12CmdList->SetComputeRoot32BitConstants(InRootParameterIndex, InNum32BitValuesToSet, InSrcData, InDestOffsetIn32BitValues);
	}

	void D3D12CommandList::DrawIndexed(uint64_t InIndexCountPerInstance)
	{

		// Now that the descriptors are in GPU we can reference the relative views in the pipeline
		m_D3D12CmdList->DrawIndexedInstanced(InIndexCountPerInstance, 1, 0, 0, 0);
	}

	void D3D12CommandList::Dispatch(uint32_t InGroupsNumX, uint32_t InGroupsNumY, uint32_t InGroupsNumZ)
	{
		// TODO commit staged descriptors for compute (but in this series of examples we are not using them)

		m_D3D12CmdList->Dispatch(InGroupsNumX, InGroupsNumY, InGroupsNumZ);
	}

	void D3D12CommandList::SetGraphicsRootTable(uint32_t InRootIndex, Mox::ConstantBufferView& InView)
	{
		m_D3D12CmdList->SetGraphicsRootDescriptorTable(InRootIndex, static_cast<Mox::D3D12ConstantBufferView&>(InView).m_GpuAllocatedRange->m_FirstGpuHandle);
	}

	void D3D12CommandList::UploadBufferData(Mox::BufferResource& DestinationBuffer, Mox::BufferResource& IntermediateBuffer, const void* InBufferData, size_t InDataSize)
	{
		// Now that both copy and dest resource are created on CPU, we can use them to update the corresponding GPU SubResource
		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = InBufferData; // Pointer to the memory block that contains the subresource data on CPU
		subresourceData.RowPitch = InDataSize; // Physical size in Bytes of the subresource data
		subresourceData.SlicePitch = subresourceData.RowPitch; // Size of each slice for the resource, since we assume only 1 slice, this corresponds to the size of the entire resource

		// Note: UpdateSubresources first uploads data in the intermediate resource, which is expected to be in shared memory (upload heap) 
		// and then transfers the content to the destination resource, most of the time in upload heap
		::UpdateSubresources(m_D3D12CmdList.Get(), static_cast<Mox::D3D12Resource&>(DestinationBuffer.GetResource()).GetInner().Get(), static_cast<Mox::D3D12Resource&>(IntermediateBuffer.GetResource()).GetInner().Get(), 0, 0, 1, &subresourceData);

	}

	void D3D12CommandList::UploadViewToGPU(Mox::ShaderResourceView& InSRV)
	{
		Mox::D3D12ShaderResourceView& d3d12SRV = static_cast<Mox::D3D12ShaderResourceView&>(InSRV);

		d3d12SRV.m_GpuAllocatedRange = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsGpuHeap().AllocateStaticRange(1, d3d12SRV.GetCPUDescHandle()); // Note: we are assuming SRV too always reference a range of 1 descriptors
	}

	void D3D12CommandList::UploadUavToGpu(Mox::UnorderedAccessView& InUav)
	{
		Mox::D3D12UnorderedAccessView& d3d12Uav = static_cast<Mox::D3D12UnorderedAccessView&>(InUav);

		d3d12Uav.m_GpuAllocatedRange = static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsGpuHeap().AllocateStaticRange(1, d3d12Uav.GetCPUDescHandle()); // Note: we are assuming Uav too always reference a range of 1 descriptors
	}

	void D3D12CommandList::ReferenceSRV(uint32_t InRootIdx, Mox::ShaderResourceView& InSRV)
	{
		// TODO this will work for graphics command list only, it would also need to work for compute... so we would need to know the type of operation we are executing...

		m_D3D12CmdList->SetGraphicsRootDescriptorTable(InRootIdx, static_cast<Mox::D3D12ShaderResourceView&>(InSRV).GetGPUDescHandle());
	}

	void D3D12CommandList::ReferenceCBV(uint32_t InRootIdx, Mox::ConstantBufferView& InCBV)
	{
		// TODO this is the same operation done in ReferenceSRV .. would it be worth to merge them as ReferenceResource(..) function?

		m_D3D12CmdList->SetGraphicsRootDescriptorTable(InRootIdx, static_cast<Mox::D3D12ConstantBufferView&>(InCBV).GetGpuDescHandle());
	}

	void D3D12CommandList::ReferenceComputeTable(uint32_t InRootIdx, Mox::UnorderedAccessView& InUav)
	{
		m_D3D12CmdList->SetComputeRootDescriptorTable(InRootIdx, static_cast<Mox::D3D12UnorderedAccessView&>(InUav).GetGPUDescHandle());
	}

	void D3D12CommandList::ReferenceComputeTable(uint32_t InRootIdx, Mox::ShaderResourceView& InUav)
	{
		m_D3D12CmdList->SetComputeRootDescriptorTable(InRootIdx, static_cast<Mox::D3D12ShaderResourceView&>(InUav).GetGPUDescHandle());
	}

	void D3D12CommandList::SetGraphicsRootDescriptorTable(uint32_t InRootIdx, D3D12_GPU_DESCRIPTOR_HANDLE InGpuDescHandle) { m_D3D12CmdList->SetGraphicsRootDescriptorTable(InRootIdx, InGpuDescHandle); }

	CD3DX12_GPU_DESCRIPTOR_HANDLE D3D12CommandList::CopyDynamicDescriptorsToBoundHeap(uint32_t InRangesNum, D3D12_CPU_DESCRIPTOR_HANDLE* InDescHandleArray, uint32_t* InRageSizeArray)
	{
		return static_cast<Mox::D3D12GraphicsAllocator*>(Mox::GraphicsAllocator::Get())->GetDescriptorsGpuHeap().CopyDynamicDescriptors(InRangesNum, InDescHandleArray, InRageSizeArray);
	}

	void D3D12CommandList::StageDynamicCbv(uint32_t InRootIndex, Mox::ConstantBufferView& InCbv)
	{
		m_StagedDescriptorManager.StageDynamicDescriptors(InRootIndex, static_cast<Mox::D3D12ConstantBufferView&>(InCbv).GetCPUDescHandle(), 1); // TODO At the moment range size hardcoded to 1
	}

	void D3D12CommandList::CommitStagedViews()
	{
		// Note: this will upload descriptors relative to descriptor tables on GPU and then reference them in the pipeline!
		m_StagedDescriptorManager.CommitStagedDescriptorsForDraw(*this);
	}

	void D3D12CommandList::D3D12StagedDescriptorManager::StageDynamicDescriptors(uint32_t InRootParamIndex, D3D12_CPU_DESCRIPTOR_HANDLE InFirstCpuDescHandle, uint32_t InRangeSize)
	{
		m_DynamicTableRootIdx[m_CurrentStagedDynamicTablesNum] = InRootParamIndex;
		m_DynamicTableFirstHandle[m_CurrentStagedDynamicTablesNum] = InFirstCpuDescHandle;
		m_DynamicRangeSizes[m_CurrentStagedDynamicTablesNum] = InRangeSize;
		
		m_CurrentStagedDynamicTablesNum++;
	}

	void D3D12CommandList::D3D12StagedDescriptorManager::StageStaticDescriptors(uint32_t InRootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE InFirstGpuDescHandle)
	{
		m_StaticTableRootIdx[m_CurrentStagedStaticTablesNum] = InRootParamIndex;
		m_StaticTableFirstHandle[m_CurrentStagedStaticTablesNum] = InFirstGpuDescHandle;

		m_CurrentStagedStaticTablesNum++;
	}

	void D3D12CommandList::D3D12StagedDescriptorManager::CommitStagedDescriptorsForDraw(D3D12CommandList& InCmdList)
	{
		CommitStagedDescriptors_Internal(InCmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
	}

	void D3D12CommandList::D3D12StagedDescriptorManager::CommitStagedDescriptors_Internal(D3D12CommandList& InCmdList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> InSetFn)
	{
		Mox::D3D12Device& d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice());
		uint32_t descSize = d3d12Device.GetCbvSrvUavDescHandleSize();

		// Uploading dynamic descriptors to GPU and adding them to the current static table entries so they can be uploaded altogether to GPU
		CD3DX12_GPU_DESCRIPTOR_HANDLE newGpuDescFirstHandle = InCmdList.CopyDynamicDescriptorsToBoundHeap(m_CurrentStagedDynamicTablesNum, &m_DynamicTableFirstHandle[0], &m_DynamicRangeSizes[0]);

		while (m_CurrentStagedDynamicTablesNum)
		{
			m_CurrentStagedDynamicTablesNum--;
			InCmdList.SetGraphicsRootDescriptorTable(m_DynamicTableRootIdx[m_CurrentStagedDynamicTablesNum], CD3DX12_GPU_DESCRIPTOR_HANDLE(newGpuDescFirstHandle, m_CurrentStagedDynamicTablesNum, descSize));
		}

		// Setting static descriptors (the ones already uploaded to GPU)
		while (m_CurrentStagedStaticTablesNum)
		{
			m_CurrentStagedStaticTablesNum--;
			InCmdList.SetGraphicsRootDescriptorTable(m_StaticTableRootIdx[m_CurrentStagedStaticTablesNum], m_StaticTableFirstHandle[m_CurrentStagedStaticTablesNum]);
		}

		// Note: m_CurrentStagedDynamicTablesNum and m_CurrentStagedStaticTablesNum are already 0 at this point, so the array are effectively reset
	}


}