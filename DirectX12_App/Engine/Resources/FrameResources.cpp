#include "FrameResources.h"

bool FrameResources::Initialize(ID3D12Device* device) {
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator));
	if (FAILED(hr)) {
		return false;
	}
	return true;
}
