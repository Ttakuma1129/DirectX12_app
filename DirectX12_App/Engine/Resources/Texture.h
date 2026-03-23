#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

class Texture {
public:
	// テクスチャリソースの作成、アップロード、SRV作成
	bool Create(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		uint32_t with,
		uint32_t height,
		const void* pixels,
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_texture; // GPU上のテクスチャ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer; // 中間バッファ
};