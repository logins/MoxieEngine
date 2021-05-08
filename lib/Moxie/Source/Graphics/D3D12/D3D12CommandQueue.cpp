/*
 D3D12CommandQueue.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "D3D12CommandQueue.h"
#include "MoxUtils.h"
#include "D3D12Device.h"
#include "D3D12UtilsInternal.h"
#include "D3D12CommandList.h"
#include "CommandList.h"
#include "D3D12MoxUtils.h"

namespace Mox {

	using namespace Microsoft::WRL;

	D3D12CommandQueue::D3D12CommandQueue(Mox::Device& InDevice, Mox::COMMAND_LIST_TYPE InCmdListType)
		: m_GraphicsDevice(InDevice)
	{
		// Note: static cast reference conversion because at this point we should be certain that the device is a D3D12 one
		Init(static_cast<Mox::D3D12Device&>(InDevice).GetInner(), CmdListTypeToD3D12(InCmdListType));
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{

		::CloseHandle(m_FenceEvent);
	}

	void D3D12CommandQueue::Init(Microsoft::WRL::ComPtr <ID3D12Device2> InDevice, D3D12_COMMAND_LIST_TYPE InCmdListType)
	{
		m_LastSeenFenceValue = 0;
		m_CmdListType = InCmdListType;
		m_Device = InDevice;

		m_CmdQueue = Mox::CreateCommandQueue(InDevice, InCmdListType);

		m_Fence = Mox::CreateFence(InDevice); // TODO change name of this fence since it is too generic
		m_GpuFrameFence = Mox::CreateFence(InDevice);
		m_FenceEvent = Mox::CreateFenceEventHandle();

		IsInitialized = true;
	}

	ComPtr<ID3D12GraphicsCommandList2> D3D12CommandQueue::GetAvailableCmdList()
	{
		// Get an available command allocator first
		ComPtr<ID3D12CommandAllocator> cmdAllocator;
		// Check first if we have an available allocator in the queue (each allocator uniquely corresponds to a different list)
		// Note: an allocator is available if the relative commands have been fully executed, 
		// so if the relative fence value has been reached by the command queue
		if (!m_CmdAllocators.empty() && IsFenceComplete(m_CmdAllocators.front().FenceValue))
		{
			cmdAllocator = m_CmdAllocators.front().CmdAllocator;
			m_CmdAllocators.pop();

			Mox::ThrowIfFailed(cmdAllocator->Reset());
		}
		else
		{
			cmdAllocator = Mox::CreateCommandAllocator(m_Device, m_CmdListType);
		}

		// Then get an available command list
		ComPtr<ID3D12GraphicsCommandList2> cmdList;
		if (!m_CmdLists.empty())
		{
			cmdList = m_CmdLists.front();
			m_CmdLists.pop();
			// Resetting the command list with the previously selected command allocator (so binding the two together)
			cmdList->Reset(cmdAllocator.Get(), nullptr);
		}
		else
		{
			cmdList = Mox::CreateCommandList(m_Device, cmdAllocator, m_CmdListType, false);
		}

		// Reference the chosen command allocator in the command list's private data, so we can retrieve it on the fly when we need it
		Mox::ThrowIfFailed(cmdList->SetPrivateDataInterface(__uuidof(cmdAllocator), cmdAllocator.Get()));
		// Note: setting a ComPtr as private data Does increment the reference count of that ComPtr !!

		return cmdList;
	}

	// Platform-agnostic version
	Mox::CommandList& D3D12CommandQueue::GetAvailableCommandList()
	{
		// Get an available command allocator first
		ComPtr<ID3D12CommandAllocator> cmdAllocator;
		// Check first if we have an available allocator in the queue (each allocator uniquely corresponds to a different list)
		// Note: an allocator is available if the relative commands have been fully executed, 
		// so if the relative fence value has been reached by the command queue
		if (!m_CmdAllocators.empty() && IsFenceComplete(m_CmdAllocators.front().FenceValue))
		{
			cmdAllocator = m_CmdAllocators.front().CmdAllocator;
			m_CmdAllocators.pop();

			Mox::ThrowIfFailed(cmdAllocator->Reset());
		}
		else
		{
			cmdAllocator = Mox::CreateCommandAllocator(m_Device, m_CmdListType);
		}

		// Then get an available command list
		if (!m_CmdListsAvailable.empty())
		{
			Mox::D3D12CommandList* outObj = static_cast<Mox::D3D12CommandList*>(m_CmdListsAvailable.front());

			auto cmdList = outObj->GetInner();
			m_CmdListsAvailable.pop();
			// Resetting the command list with the previously selected command allocator (so binding the two together)
			cmdList->Reset(cmdAllocator.Get(), nullptr);
			// Reference the chosen command allocator in the command list's private data, so we can retrieve it on the fly when we need it
			Mox::ThrowIfFailed(outObj->GetInner()->SetPrivateDataInterface(__uuidof(cmdAllocator), cmdAllocator.Get()));
			// Note: setting a ComPtr as private data Does increment the reference count of that ComPtr !!
			return *outObj;
		}
		
		// If here, we need to create a new command list
		m_CmdListPool.emplace(std::make_unique<Mox::D3D12CommandList>(Mox::CreateCommandList(m_Device, cmdAllocator, m_CmdListType, false), m_GraphicsDevice));
		Mox::D3D12CommandList& outObj = *static_cast<Mox::D3D12CommandList*>(m_CmdListPool.back().get());
		
		Mox::ThrowIfFailed(outObj.GetInner()->SetPrivateDataInterface(__uuidof(cmdAllocator), cmdAllocator.Get()));
		
		return outObj;
	}

	uint64_t D3D12CommandQueue::ExecuteCmdList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> InCmdList)
	{
		InCmdList->Close();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
		UINT dataSize = sizeof(cmdAllocator);
		Mox::ThrowIfFailed(InCmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, cmdAllocator.GetAddressOf()));

		ID3D12CommandList* const ppCmdLists[] = { InCmdList.Get() };

		m_CmdQueue->ExecuteCommandLists(1, ppCmdLists);
		uint64_t fenceValue = Signal();

		m_CmdAllocators.emplace( CmdAllocatorEntry{ fenceValue, cmdAllocator } ); // Note: implicit creation of a ComPtr from a raw pointer to create CmdAllocatorEntry
		m_CmdLists.push(InCmdList);

		return fenceValue;
	}

	// Platform-agnostic version
	uint64_t D3D12CommandQueue::ExecuteCmdList(Mox::CommandList& InCmdList)
	{
		InCmdList.Close();

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
		UINT dataSize = sizeof(cmdAllocator);

		auto d3d12CmdList = static_cast<Mox::D3D12CommandList&>(InCmdList).GetInner();

		Mox::ThrowIfFailed(d3d12CmdList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, cmdAllocator.GetAddressOf()));

		ID3D12CommandList* const ppCmdLists[] = { d3d12CmdList.Get() };

		m_CmdQueue->ExecuteCommandLists(1, ppCmdLists);
		uint64_t fenceValue = Signal();

		m_CmdAllocators.emplace(CmdAllocatorEntry{ fenceValue, cmdAllocator }); // Note: implicit creation of a ComPtr from a raw pointer to create CmdAllocatorEntry
		
		m_CmdListsAvailable.push(&InCmdList);

		return fenceValue;
	}

	uint64_t D3D12CommandQueue::Signal()
	{
		Mox::SignalCmdQueue(m_CmdQueue, m_Fence, m_LastSeenFenceValue);
		return m_LastSeenFenceValue;
	}

	bool D3D12CommandQueue::IsFenceComplete(uint64_t InFenceValue)
	{
		return m_Fence->GetCompletedValue() >= InFenceValue;
	}

	void D3D12CommandQueue::WaitForFenceValue(uint64_t InFenceValue)
	{
		Mox::WaitForFenceValue(m_Fence, InFenceValue, m_FenceEvent);
	}

	void D3D12CommandQueue::Flush()
	{
		Mox::FlushCmdQueue(m_CmdQueue, m_Fence, m_FenceEvent, m_LastSeenFenceValue);
	}

	void D3D12CommandQueue::OnRenderFrameStarted()
	{

	}

	void D3D12CommandQueue::OnRenderFrameFinished()
	{
		m_CompletedRenderFrames++;

		// When a render frame finishes, we signal it in the command queue work, so we can figure out later when Gpu work finished executing for a particular frame
		ThrowIfFailed(m_CmdQueue->Signal(m_GpuFrameFence.Get(), m_CompletedRenderFrames));
	}

	uint64_t D3D12CommandQueue::ComputeFramesInFlightNum()
	{
		return m_CompletedRenderFrames - m_GpuFrameFence->GetCompletedValue();
	}

	void D3D12CommandQueue::WaitForGpuFrames(uint64_t InFramesToWait)
	{
		Check(m_GpuFrameFence->GetCompletedValue() + InFramesToWait <= m_CompletedRenderFrames)

		Mox::WaitForFenceValue(m_GpuFrameFence, m_GpuFrameFence->GetCompletedValue() + InFramesToWait, m_FenceEvent);
	}

}
