#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <DirectXMath.h>

// シーン全体で共通(VP行列)
struct alignas(256) SceneConstant {
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
	DirectX::XMFLOAT4 lightDir; // ライトの方向
	DirectX::XMFLOAT4 lightColor; // ライトの色
	DirectX::XMFLOAT4 ambientColor; // 環境色
	DirectX::XMFLOAT4 cameraPos; // カメラ位置
	DirectX::XMFLOAT4 specularParams; // x:強度 y:鋭さ
	DirectX::XMFLOAT4X4 lightViewProj; // ライトのVP行列
	DirectX::XMFLOAT4 shadowParams; // x:偏り y:シャドウマップのサイズ
	DirectX::XMFLOAT4 fogColor; // rgb:色, a:未使用
	DirectX::XMFLOAT4 fogParams; // x:開始距離 y:終了距離
};

// オブジェクトごと(M行列)
struct alignas(256) ObjectConstant {
	DirectX::XMFLOAT4X4 model;
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