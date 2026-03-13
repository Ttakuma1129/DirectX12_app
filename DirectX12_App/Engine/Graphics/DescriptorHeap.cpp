#include "DescriptorHeap.h"

bool DescriptorHeap::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible){
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	
	heapDesc.Type = type;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = numDescriptors;
	// shaderVisible‚ªtrue‚È‚çSHADER_VISIBLE‚É,false‚È‚çNONE‚É‚·‚é
	heapDesc.Flags = shaderVisible 
						? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
						: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device -> CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
	if (FAILED(hr)) {
		return false;
	}
	m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);
	m_numDescriptors = numDescriptors;
	return true;
}