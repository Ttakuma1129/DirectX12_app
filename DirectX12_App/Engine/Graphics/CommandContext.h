#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class CommandContext {
public:
	bool Initialize(ID3D12Device* device, ID3D12CommandAllocator* allocator);

	bool Begin(ID3D12CommandAllocator* allocator);

	bool TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	bool ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float color[4]);

	bool End();

	bool Execute(ID3D12CommandQueue* commandQueue);

	ID3D12GraphicsCommandList* GetCommandList() {
		return m_commandList.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
};