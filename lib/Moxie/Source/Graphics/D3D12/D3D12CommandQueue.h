/*
 D3D12CommandQueue.h

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/

#ifndef D3D12CommandQueue_h__
#define D3D12CommandQueue_h__

#include <wrl.h>
#include <d3d12.h>
#include <queue> // For std::queue
#include <memory>
#include "CommandQueue.h"

namespace Mox {

	class D3D12CommandQueue : public Mox::CommandQueue
	{
	public:

		D3D12CommandQueue(class Mox::Device& InDevice, Mox::COMMAND_LIST_TYPE InCmdListType);

		~D3D12CommandQueue();

		void Init(Microsoft::WRL::ComPtr <ID3D12Device2> InDevice, D3D12_COMMAND_LIST_TYPE InCmdListType);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetAvailableCmdList();

		// Platform-agnostic version
		virtual Mox::CommandList& GetAvailableCommandList() override;

		uint64_t ExecuteCmdList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> InCmdList);

		// Platform-agnostic version
		virtual uint64_t ExecuteCmdList(Mox::CommandList& InCmdList) override;


		uint64_t Signal();
		bool IsFenceComplete(uint64_t InFenceValue);
		void WaitForFenceValue(uint64_t InFenceValue);
		// Signals the fence and stalls the thread it is invoked on to wait for the just signaled fence value
		virtual void Flush();

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CmdQueue() const { return m_CmdQueue; }

		Microsoft::WRL::ComPtr<ID3D12Device2> GetD3D12Device() const { return m_Device; };

		virtual void OnRenderFrameStarted() override;

		virtual void OnRenderFrameFinished() override;

		virtual uint64_t ComputeFramesInFlightNum() override;

		virtual void WaitForGpuFrames(uint64_t InFramesToWaitNum) override;

	private:
		uint64_t m_CompletedGPUFramesNum = 0;

		using CmdListQueue = std::queue<std::unique_ptr<Mox::CommandList>>;
		// Note: references are objects that are not copyable hence we cannot use them for containers and need to store pointers
		using CmdListQueueRefs = std::queue<Mox::CommandList*>;

		CmdListQueue m_CmdListPool;
		CmdListQueueRefs m_CmdListsAvailable;

		// Platform-agnostic reference to the device that holds this command queue
		Mox::Device& m_GraphicsDevice;

		// Keep track of command allocators in current execution
		struct CmdAllocatorEntry
		{
			uint64_t FenceValue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdAllocator;
		};
		using D3D12CmdAllocatorQueue = std::queue<CmdAllocatorEntry>;

		using D3D12CmdListQueue = std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>>;

		D3D12_COMMAND_LIST_TYPE m_CmdListType;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CmdQueue;
		Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		// Used exclusively to keep track of the completed frames on Gpu
		Microsoft::WRL::ComPtr<ID3D12Fence> m_GpuFrameFence;
		HANDLE m_FenceEvent;
		uint64_t m_LastSeenFenceValue = 0;
		uint64_t m_CompletedRenderFrames = 0;
		D3D12CmdAllocatorQueue m_CmdAllocators;
		D3D12CmdListQueue m_CmdLists;

		bool IsInitialized = false;
	};

}
#endif // D3D12CommandQueue_h__
