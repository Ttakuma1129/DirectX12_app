#pragma once
#include <d3d12.h>
#include <wrl/client.h>

class RootSignature;

class PipelineState {
public:
	bool Initialize(
		ID3D12Device* device,
		ID3D12RootSignature* rootSignature,
		const void* vsData, size_t vsSize, // 頂点シェーダーバイトサイズ
		const void* psData, size_t psSize // ピクセルシェーダーバイトサイズ
	);

	ID3D12PipelineState* GetPipelineState() const {
		return m_pipelineState.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};