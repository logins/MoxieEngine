/*
 D3D12CommandList.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef D3D12CommandList_h__
#define D3D12CommandList_h__

#include "CommandList.h"
#include <wrl.h>
#include <functional>
#include "d3dx12.h"

namespace Mox { 

	class D3D12Device;

	class D3D12CommandList : public Mox::CommandList
	{
	public:
		D3D12CommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> InCmdList, Mox::Device& InOwningDevice);
		
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>& GetInner() { return m_D3D12CmdList; }

		virtual void ResourceBarrier(Mox::Resource& InResource, Mox::RESOURCE_STATE InPrevState, Mox::RESOURCE_STATE InAfterState) override;


		virtual void ClearRTV(Mox::CpuDescHandle& InDescHandle, float* InColor) override;


		virtual void ClearDepth(Mox::CpuDescHandle& InDescHandle) override;


		virtual void Close() override;

		virtual void SetPipelineStateAndResourceBinder(Mox::PipelineState& InPipelineState) override;


		virtual void SetInputAssemblerData(Mox::PRIMITIVE_TOPOLOGY InPrimTopology, Mox::VertexBufferView& InVertexBufView, Mox::IndexBufferView& InIndexBufView) override;


		virtual void SetViewportAndScissorRect(Mox::ViewPort& InViewport, Mox::Rect& InScissorRect) override;


		virtual void SetRenderTargetFromWindow(Mox::Window& InWindow) override;


		virtual void SetGraphicsRootConstants(uint64_t InRootParameterIndex, uint64_t InNum32BitValuesToSet, const void* InSrcData, uint64_t InDestOffsetIn32BitValues) override;

		virtual void SetComputeRootConstants(uint64_t InRootParameterIndex, uint64_t InNum32BitValuesToSet, const void* InSrcData, uint64_t InDestOffsetIn32BitValues) override;


		virtual void DrawIndexed(uint64_t InIndexCountPerInstance) override;

		virtual void Dispatch(uint32_t InGroupsNumX, uint32_t InGroupsNumY, uint32_t InGroupsNumZ) override;

		virtual void SetGraphicsRootTable(uint32_t InRootIndex, Mox::ConstantBufferView& InView) override;

	
		virtual void UploadBufferData(Mox::Buffer& DestinationBuffer, Mox::Buffer& IntermediateBuffer, const void* InBufferData, size_t InDataSize) override;


		virtual void UploadViewToGPU(Mox::ShaderResourceView& InSRV) override;

		virtual void UploadUavToGpu(Mox::UnorderedAccessView& InUav) override;

		virtual void ReferenceSRV(uint32_t InRootIdx, Mox::ShaderResourceView& InSRV) override;

		virtual void ReferenceCBV(uint32_t InRootIdx, Mox::ConstantBufferView& InCBV) override;

		virtual void ReferenceComputeTable(uint32_t InRootIdx, Mox::ShaderResourceView& InUav) override;

		virtual void ReferenceComputeTable(uint32_t InRootIdx, Mox::UnorderedAccessView& InUav) override;


		void SetGraphicsRootDescriptorTable(uint32_t InRootIdx, D3D12_GPU_DESCRIPTOR_HANDLE InGpuDescHandle);

		CD3DX12_GPU_DESCRIPTOR_HANDLE CopyDynamicDescriptorsToBoundHeap(uint32_t InTablesNum, D3D12_CPU_DESCRIPTOR_HANDLE* InDescHandleArray, uint32_t* InRageSizeArray);


		void StageDynamicCbv(uint32_t InRootIndex, Mox::ConstantBufferView& InCbv) override;


		void CommitStagedViews() override;

	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_D3D12CmdList;

		class D3D12StagedDescriptorManager {
		public:
			// Dynamic entries will be first uploaded to the desc heap bound to the root signature, and then bound to the command list as root table when the next draw/dispatch command is executed
			void StageDynamicDescriptors(uint32_t InRootParamIndex, D3D12_CPU_DESCRIPTOR_HANDLE InFirstCpuDescHandle, uint32_t InRangeSize);

			// Static entries are already allocated in GPU and they will be directly bound to the command list as root table when the next draw/dispatch command is executed
			void StageStaticDescriptors(uint32_t InRootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE InFirstGpuDescHandle);

			void CommitStagedDescriptorsForDraw(D3D12CommandList& InCmdList);

		private:
			void CommitStagedDescriptors_Internal(D3D12CommandList& InCmdList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> InSetFn);

			uint32_t m_DynamicTableRootIdx[32];
			D3D12_CPU_DESCRIPTOR_HANDLE m_DynamicTableFirstHandle[32];
			uint32_t m_DynamicRangeSizes[32];
			uint32_t m_CurrentStagedDynamicTablesNum = 0;

			uint32_t m_StaticTableRootIdx[32];
			D3D12_GPU_DESCRIPTOR_HANDLE m_StaticTableFirstHandle[32];
			uint32_t m_CurrentStagedStaticTablesNum = 0;
		};
		D3D12StagedDescriptorManager m_StagedDescriptorManager;

	};

	}
#endif // D3D12CommandList_h__
