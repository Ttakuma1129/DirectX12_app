#include "Mesh.h"

bool Mesh::Create(ID3D12Device* device, const void* vertices, uint32_t vertexSize, uint32_t stride, const uint16_t* indices, uint32_t indexCount) {

	HRESULT hr;

	// インデックスデータ
	uint16_t indices[] = {
		// 手前 (z = -0.5)
		0, 1, 2,
		2, 1, 3,
		// 左 (z = 0.5)
		4, 5, 6,
		6, 5, 7,
		// 右 (x = -0.5)
		8, 9, 10,
		10, 9, 11,
		// 奥 (x = 0.5)
		13, 12, 15,
		15, 12, 14,
		// 下 (y = 0.5)
		16, 17, 18,
		18, 17, 19,
		// 上 (y = -0.5)
		20, 21, 22,
		22, 21, 23,
	};

	// 頂点バッファの作成
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(vertices);
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// 頂点バッファをGPUに転送(マッピング)
	void* mapped = nullptr;
	hr = m_vertexBuffer->Map(0, nullptr, &mapped);
	if (FAILED(hr)) {
		return false;
	}

	memcpy(mapped, vertices, sizeof(vertices));
	m_vertexBuffer->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(vertices); // バッファ全体のサイズ
	m_vertexBufferView.StrideInBytes = sizeof(vertexSize);	// 1頂点のバッファサイズ

	// インデックスバッファの作成
	D3D12_HEAP_PROPERTIES ibHeapProps = {};
	ibHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC ibResDesc = {};
	ibResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ibResDesc.Width = sizeof(indices);
	ibResDesc.Height = 1;
	ibResDesc.DepthOrArraySize = 1;
	ibResDesc.MipLevels = 1;
	ibResDesc.Format = DXGI_FORMAT_UNKNOWN;
	ibResDesc.SampleDesc.Count = 1;
	ibResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = device->CreateCommittedResource(
		&ibHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&ibResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// インデックスバッファをGPUに転送(マッピング)
	void* ibMapped = nullptr;
	hr = m_indexBuffer->Map(0, nullptr, &ibMapped);
	if (FAILED(hr)) {
		return false;
	}

	memcpy(ibMapped, indices, sizeof(indices));
	m_indexBuffer->Unmap(0, nullptr);

	// インデックスバッファビューの作成
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(indices);
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	m_indexCount = indexCount;

	return true;
}

void Mesh::Bind(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmdList->IASetIndexBuffer(&m_indexBufferView);
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) {
	cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}