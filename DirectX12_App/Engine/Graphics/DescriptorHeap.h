#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class DescriptorHeap {
public:
	bool Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible);
	
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) {
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) {
		D3D12_GPU_DESCRIPTOR_HANDLE handle = m_heap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<SIZE_T>(index) * m_descriptorSize;
		return handle;
	}

	ID3D12DescriptorHeap* GetHeap() const {
		return m_heap.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	uint32_t m_descriptorSize = 0;
	uint32_t m_numDescriptors = 0;
};