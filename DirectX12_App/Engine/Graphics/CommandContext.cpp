#include "CommandContext.h"

bool CommandContext::Initialize(ID3D12Device* device, ID3D12CommandAllocator* allocator) {
	HRESULT hr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&m_commandList));

	if (FAILED(hr)) {
		return false;
	}

	m_commandList->Close();

	return true;
}

bool CommandContext::Begin(ID3D12CommandAllocator* allocator) {
	HRESULT hr;
	hr = allocator->Reset();
	if (FAILED(hr)) {
		return false;
	}

	hr = m_commandList->Reset(allocator, nullptr);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool CommandContext::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &barrier);

	return true;
}

bool CommandContext::ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const float color[4]) {
	m_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	return true;
}

bool CommandContext::End() {
	HRESULT hr = m_commandList->Close();
	if (FAILED(hr)) {
		return false;
	}
	return true;
}

bool CommandContext::Execute(ID3D12CommandQueue* commandQueue) {
	ID3D12CommandList* lists[] = { m_commandList.Get() };
	commandQueue->ExecuteCommandLists(1, lists);
	return true;
}