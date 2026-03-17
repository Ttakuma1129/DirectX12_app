#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <DirectXMath.h>

struct alignas(256) SceneConstant {
	DirectX::XMFLOAT4X4 mvp;
};

class FrameResources{
public:
	// コマンドアロケータを初期化・作成
	bool Initialize(ID3D12Device* device);

	// 外部からコマンドアロケータを取得するためのゲッター
	ID3D12CommandAllocator* GetAllocator() {
		return m_allocator.Get();
	}

	// 外部から定数バッファを取得するためのゲッター
	ID3D12Resource* GetConstantBuffer() {
		return m_constantBuffer.Get();
	}

	// 外部から定数マップを取得するためのゲッター
	SceneConstant* GetConstantMapped() {
		return m_mapped;
	}

	uint64_t fenceValue = 0;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator; //コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer; // 定数バッファ
	SceneConstant* m_mapped = nullptr; // 常時マッピング
};