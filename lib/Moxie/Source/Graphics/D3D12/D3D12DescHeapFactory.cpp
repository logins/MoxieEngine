/*
 D3D12DescHeapFactory.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
 
#include "D3D12DescHeapFactory.h"
#include "D3D12Device.h"
#include "RangeAllocators.h"
#include "D3D12MoxUtils.h"

namespace Mox { 

	D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE InType, bool IsShaderVisible, uint32_t InDescSize, uint32_t InDescriptorsNum /*= 256*/, float InStaticDescPercentage /*= 1.0f*/) 
		: m_Type(InType), m_IsShaderVisible(IsShaderVisible), m_DescriptorsNum(InDescriptorsNum), m_DescSize(InDescSize)
	{
		// Allocate D3D12 Heap
		ID3D12Device2* d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner().Get();

		// Allocate desc heap for the current page
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = InType;
		heapDesc.NumDescriptors = InDescriptorsNum;
		if(IsShaderVisible)
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Allocating it on GPU!
		Mox::ThrowIfFailed(d3d12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_D3D12DescHeap)));

		m_FirstCpuDesc = GetInner()->GetCPUDescriptorHandleForHeapStart();
		m_FirstGpuDesc = GetInner()->GetGPUDescriptorHandleForHeapStart();

		// Set allocators
		int32_t staticAllocatorSize = InDescriptorsNum * InStaticDescPercentage;
		m_StaticDescAllocator = std::make_unique<Mox::StaticRangeAllocator>(InDescriptorsNum - staticAllocatorSize, staticAllocatorSize);
		
		m_DynamicDescAllocator = std::make_unique<Mox::LinearRangeAllocator>(0, std::max( 0, static_cast<int32_t>(InDescriptorsNum) - staticAllocatorSize - 1));
	}

	D3D12DescriptorHeap::~D3D12DescriptorHeap() = default; // Defining the destructor in source will prevent the compiler to make it inline, and consequently inline the unique_ptr object members as well !! Otherwise unique_ptr type could not be forward declared!

	void D3D12DescriptorHeap::SetAllowedDynamicAllocationRegion(float InStartPercentage, float InEndPercentage)
	{
		static_cast<Mox::LinearRangeAllocator*>(m_DynamicDescAllocator.get())->SetAdmittedAllocationRegion(InStartPercentage, InEndPercentage);
	}

	std::unique_ptr<Mox::StaticDescAllocation> D3D12DescriptorHeap::AllocateStaticRange(uint32_t InRangeSize)
	{
		uint32_t descOffsetScaledByIncrementSize = m_StaticDescAllocator->AllocateRange(InRangeSize) * m_DescSize;
		if (m_IsShaderVisible) // If shader visible, setting the GPU pointer as well
			return std::make_unique<StaticDescAllocation>(*this, DescAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_FirstCpuDesc, descOffsetScaledByIncrementSize), InRangeSize, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_FirstGpuDesc, descOffsetScaledByIncrementSize)));

		return std::make_unique<StaticDescAllocation>(*this, DescAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_FirstCpuDesc, descOffsetScaledByIncrementSize), InRangeSize));
	}

	std::unique_ptr<Mox::StaticDescAllocation> D3D12DescriptorHeap::AllocateStaticRange(uint32_t InRangeSize, D3D12_CPU_DESCRIPTOR_HANDLE InStartingCpuHandleToCopyFrom)
	{
		std::unique_ptr<Mox::StaticDescAllocation> outputRange = AllocateStaticRange(InRangeSize);
		// Copy over descriptors
		auto device = static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner();
		device->CopyDescriptorsSimple(InRangeSize, outputRange->m_FirstCpuHandle, InStartingCpuHandleToCopyFrom, m_Type);

		return outputRange;
	}

	void D3D12DescriptorHeap::FreeAllocatedStaticRange(const D3D12_CPU_DESCRIPTOR_HANDLE& InFirstCpuHandle, uint32_t InRangeSize)
	{
		m_StaticDescAllocator->FreeAllocatedRange(CpuDescToAllocatorOffset(InFirstCpuHandle), InRangeSize);
	}

	Mox::DescAllocation D3D12DescriptorHeap::AllocateDynamicRange(uint32_t InRangeSize)
	{
		uint32_t descOffsetScaledByIncrementSize = m_DynamicDescAllocator->AllocateRange(InRangeSize) * m_DescSize;
		if(m_IsShaderVisible) // If shader visible, setting the GPU pointer as well
			return DescAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_FirstCpuDesc, descOffsetScaledByIncrementSize), InRangeSize, CD3DX12_GPU_DESCRIPTOR_HANDLE(m_FirstGpuDesc, descOffsetScaledByIncrementSize));

		return DescAllocation(CD3DX12_CPU_DESCRIPTOR_HANDLE(m_FirstCpuDesc, descOffsetScaledByIncrementSize), InRangeSize);
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::CopyDynamicDescriptors(uint32_t InRangesNum, D3D12_CPU_DESCRIPTOR_HANDLE* InDescHandleArray, uint32_t InRageSizeArray[])
	{
		Mox::D3D12Device& d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice());
		// Compute total number of descriptors to copy
		int32_t currentRangeIdx = InRangesNum - 1;
		uint32_t totalDescriptorsNum = 0;
		while (currentRangeIdx >= 0)
		{
			totalDescriptorsNum += InRageSizeArray[currentRangeIdx];
			currentRangeIdx--;
		}
		// Reserve offset in the dynamic allocator to contain all the descriptor handles
		uint32_t firstDescHandleOffset = m_DynamicDescAllocator->AllocateRange(totalDescriptorsNum);
		// Copy descriptors: we copy all the ranges, one after the other, so the destination range is going to be a single big one
		CD3DX12_CPU_DESCRIPTOR_HANDLE destFirstDescHandle(m_FirstCpuDesc, firstDescHandleOffset, m_DescSize); 
		// Note: we are copying into the CPU side of the heap, but due to mapping, the GPU heap will be updated consequently
		d3d12Device.CopyDescriptors(1, &destFirstDescHandle, &totalDescriptorsNum, InRangesNum, InDescHandleArray, InRageSizeArray, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_FirstGpuDesc, firstDescHandleOffset, m_DescSize);
	}

	uint32_t D3D12DescriptorHeap::CpuDescToAllocatorOffset(const D3D12_CPU_DESCRIPTOR_HANDLE& InCpuHandle)
	{
		return (InCpuHandle.ptr - m_FirstCpuDesc.ptr) / m_DescSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::AllocatorOffsetToCpuDesc(uint32_t InOffset)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_FirstCpuDesc, InOffset, m_DescSize);
	}

	D3D12DescHeapFactory::D3D12DescHeapFactory()
	{
		auto d3d12Device = static_cast<Mox::D3D12Device&>(Mox::GetDevice()).GetInner().Get();

		uint32_t descriptorSize = d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		
		m_CPUDescHeap = std::make_unique<D3D12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, descriptorSize);
		m_GPUDescHeap = std::make_unique<D3D12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true, descriptorSize, 256, 0.5f);

		m_ResourceViewArray.reserve(100);
	}

	D3D12DescHeapFactory::~D3D12DescHeapFactory()
	{
		// Views need to go first, before heaps get destroyed
		m_ResourceViewArray.clear();
	}
	
	Mox::D3D12DescriptorHeap& D3D12DescHeapFactory::GetCPUHeap()
	{
		return *m_CPUDescHeap;
	}

	Mox::D3D12DescriptorHeap& D3D12DescHeapFactory::GetGPUHeap()
	{
		return *m_GPUDescHeap;
	}

	Mox::ResourceView& D3D12DescHeapFactory::AddViewObject(std::unique_ptr<Mox::ResourceView> InResourceView)
	{
		m_ResourceViewArray.push_back(std::move(InResourceView));
		return *m_ResourceViewArray.back();
	}

	StaticDescAllocation::~StaticDescAllocation()
	{
		m_DescHeap.FreeAllocatedStaticRange(m_FirstCpuHandle, m_RangeSize);
	}

}
