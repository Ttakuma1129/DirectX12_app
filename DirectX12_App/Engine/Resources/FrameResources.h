#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class FrameResources{
public:
	// コマンドアロケータを初期化・作成
	bool Initialize(ID3D12Device* device);

	// 外部からコマンドアロケータを取得するためのゲッター
	ID3D12CommandAllocator* GetAllocator() {
		return m_allocator.Get();
	}

	uint64_t fenceValue = 0;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator; //コマンドアロケータ
};