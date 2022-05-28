/*
 CommandList.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef CommandList_h__
#define CommandList_h__

#include "GraphicsTypes.h"

namespace Mox { 

	class Device;
	class PipelineState;
	class Window;

	using TransitionInfoVector = std::vector<std::tuple<Mox::Resource*, Mox::RESOURCE_STATE, Mox::RESOURCE_STATE>>;

	class CommandList
	{
	public:

		virtual ~CommandList() = default;

		virtual void Close() = 0;

		virtual void ResourceBarriers(const TransitionInfoVector& InTransitions) = 0;

		virtual void ClearRTV(Mox::CpuDescHandle& InDescHandle, float* InColor) = 0;
		
		virtual void ClearDepth(Mox::CpuDescHandle& InDescHandle) = 0;

		virtual void SetPipelineStateAndResourceBinder(Mox::PipelineState& InPipelineState) = 0;

		virtual void SetInputAssemblerData(Mox::PRIMITIVE_TOPOLOGY InPrimTopology, Mox::VertexBufferView& InVertexBufView, Mox::IndexBufferView& InIndexBufView) = 0;

		virtual void SetViewportAndScissorRect(Mox::ViewPort& InViewport, Mox::Rect& InScissorRect) = 0;

		virtual void SetRenderTargetFromWindow(Mox::Window& InWindow) = 0;

		virtual void SetGraphicsRootConstants(uint64_t InRootParameterIndex, uint64_t InNum32BitValuesToSet, const void* InSrcData, uint64_t InDestOffsetIn32BitValues) = 0;

		virtual void SetComputeRootConstants(uint64_t InRootParameterIndex, uint64_t InNum32BitValuesToSet, const void* InSrcData, uint64_t InDestOffsetIn32BitValues) = 0;

		virtual void SetGraphicsRootTable(uint32_t InRootIndex, Mox::ConstantBufferView& InView) = 0;

		// Records the given Cbv at the specified root index for Gpu upload upon calling CommitStagedDescriptors()
		virtual void StageDynamicCbv(uint32_t InRootIndex, Mox::ConstantBufferView& InCbv) = 0;

		// Uploads the staged descriptors to Gpu. This is usually done before a draw command using such descriptors.
		virtual void CommitStagedViews() = 0;

		virtual void DrawIndexed(uint64_t InIndexCountPerInstance) = 0;

		virtual void Dispatch(uint32_t InGroupsNumX, uint32_t InGroupsNumY, uint32_t InGroupsNumZ) = 0;

		virtual void UploadViewToGPU(Mox::ShaderResourceView& InSRV) = 0;

		virtual void UploadUavToGpu(Mox::UnorderedAccessView& InUav) = 0;

		virtual void ReferenceSRV(uint32_t InRootIdx, Mox::ShaderResourceView& InSRV) = 0;

		virtual void ReferenceCBV(uint32_t InRootIdx, Mox::ConstantBufferView& InCBV) = 0;

		virtual void ReferenceComputeTable(uint32_t InRootIdx, Mox::ShaderResourceView& InUav) = 0;

		virtual void ReferenceComputeTable(uint32_t InRootIdx, Mox::UnorderedAccessView& InUav) = 0;

		// Internally calls ::UpdateSubresources(..) where IntermediateBuffer is expected to be allocated in upload heap
		virtual void UploadBufferData(Mox::BufferResource& DestinationBuffer, Mox::BufferResource& IntermediateBuffer, const void* InBufferData, size_t InDataSize) = 0;


	protected:
		CommandList(Mox::Device& InDevice);

		Mox::Device& m_Device;
	};

}

#endif // CommandList_h__
