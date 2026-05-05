#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>


class  Mesh{
public:
	bool Create(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vertices, uint32_t vertexSize, uint32_t stride, const uint16_t* indices, uint32_t indexCount);

	void ReleaseUploadBuffer();

	void Bind(ID3D12GraphicsCommandList* cmdList);

	void Draw(ID3D12GraphicsCommandList* cmdList);

	bool IsEmpty() const {
		return m_indexCount == 0;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer; // 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer; // インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vbUploadBuffer; // 中間バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ibUploadBuffer;

	// バッファビュー
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	uint32_t m_indexCount = 0;
};