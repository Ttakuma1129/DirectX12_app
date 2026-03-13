#pragma once
#include <d3d12.h>
#include <wrl/client.h>

class RootSignature {
public:
	bool Initialize(ID3D12Device* device);

	ID3D12RootSignature* GetRootSignature() const {
		return m_rootSignature.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
};