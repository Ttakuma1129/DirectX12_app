#include "Mesh.h"

bool Mesh::Create(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vertices, uint32_t vertexSize, uint32_t stride, const uint16_t* indices, uint32_t indexCount) {

	HRESULT hr;
	
	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_HEAP_PROPERTIES defaultHeapProps = {};
	defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 頂点バッファの設定
	D3D12_RESOURCE_DESC vbDesc = {};
	vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vbDesc.Width = vertexSize;
	vbDesc.Height = 1;
	vbDesc.DepthOrArraySize = 1;
	vbDesc.MipLevels = 1;
	vbDesc.Format = DXGI_FORMAT_UNKNOWN;
	vbDesc.SampleDesc.Count = 1;
	vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 頂点バッファの作成
	hr = device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// アップロード用中間バッファの作成
	hr = device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vbUploadBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// 頂点バッファをGPUに転送(マッピング)
	void* mapped = nullptr;
	hr = m_vbUploadBuffer->Map(0, nullptr, &mapped);
	if (FAILED(hr)) {
		return false;
	}

	memcpy(mapped, vertices, vertexSize);
	m_vbUploadBuffer->Unmap(0, nullptr);

	// アップロードからデフォルトヒープへのコピーを記録
	cmdList->CopyBufferRegion(m_vertexBuffer.Get(), 0, m_vbUploadBuffer.Get(), 0, vertexSize);

	// COPY_DEST → VERTEX_AND_CONSTANT_BUFFERへバリア遷移
	D3D12_RESOURCE_BARRIER vbBarrier = {};
	vbBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	vbBarrier.Transition.pResource = m_vertexBuffer.Get();
	vbBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	vbBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	vbBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdList->ResourceBarrier(1, &vbBarrier);

	// 頂点バッファビューの作成
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexSize; // バッファ全体のサイズ
	m_vertexBufferView.StrideInBytes = stride;	// 1頂点のバッファサイズ

	// インデックスバッファの設定
	uint32_t indexBufferSize = indexCount * sizeof(uint16_t);

	D3D12_RESOURCE_DESC ibResDesc = {};
	ibResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ibResDesc.Width = indexBufferSize;
	ibResDesc.Height = 1;
	ibResDesc.DepthOrArraySize = 1;
	ibResDesc.MipLevels = 1;
	ibResDesc.Format = DXGI_FORMAT_UNKNOWN;
	ibResDesc.SampleDesc.Count = 1;
	ibResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// インデックスバッファの作成
	hr = device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&ibResDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// アップロード用中間バッファの作成
	hr = device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&ibResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_ibUploadBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// インデックスバッファをGPUに転送(マッピング)
	void* ibMapped = nullptr;
	hr = m_ibUploadBuffer->Map(0, nullptr, &ibMapped);
	if (FAILED(hr)) {
		return false;
	}

	memcpy(ibMapped, indices, indexBufferSize);
	m_ibUploadBuffer->Unmap(0, nullptr);

	// アップロードからデフォルトヒープへのコピーを記録
	cmdList->CopyBufferRegion(m_indexBuffer.Get(), 0, m_ibUploadBuffer.Get(), 0, indexBufferSize);

	// COPY_DEST → STATE_INDEX_BUFFERへバリア遷移
	D3D12_RESOURCE_BARRIER ibBarrier = vbBarrier;
	ibBarrier.Transition.pResource = m_indexBuffer.Get();
	ibBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	cmdList->ResourceBarrier(1, &ibBarrier);

	// インデックスバッファビューの作成
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = indexBufferSize;
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	m_indexCount = indexCount;

	return true;
}

bool Mesh::CreateDeferred(ID3D12Device* device, const void* vertices, uint32_t vertexSize, uint32_t stride, const uint16_t* indices, uint32_t indexCount) {
	
	HRESULT hr;
	
	D3D12_HEAP_PROPERTIES uploadHeap = {};
	uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_HEAP_PROPERTIES defaultHeap = {};
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

	auto bufferDesc = [](uint64_t bytes) {
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = bytes;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		return desc;
	};

	// 頂点バッファ作成
	D3D12_RESOURCE_DESC vbDesc = bufferDesc(vertexSize);
	hr = device->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	if (FAILED(hr)) {
		return false;
	}
	hr = device->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vbUploadBuffer));
	if (FAILED(hr)) {
		return false;
	}

	void* mapped = nullptr;
	hr = m_vbUploadBuffer->Map(0, nullptr, &mapped);
	if (FAILED(hr)) {
		return false;
	}
	memcpy(mapped, vertices, vertexSize);
	m_vbUploadBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexSize;
	m_vertexBufferView.StrideInBytes = stride;

	// インデックスバッファ作成
	uint32_t ibSize = indexCount * sizeof(uint16_t);
	D3D12_RESOURCE_DESC ibDesc = bufferDesc(ibSize);
	hr = device->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&ibDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer));
	if (FAILED(hr)) {
		return false;
	}
	hr = device->CreateCommittedResource(
		&uploadHeap,
		D3D12_HEAP_FLAG_NONE,
		&ibDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_ibUploadBuffer));
	if (FAILED(hr)) {
		return false;
	}

	hr = m_ibUploadBuffer->Map(0, nullptr, &mapped);
	if (FAILED(hr)) {
		return false;
	}
	memcpy(mapped, indices, ibSize);
	m_ibUploadBuffer->Unmap(0, nullptr);

	m_indexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = ibSize;
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	m_indexCount = indexCount;
	return true;
}

void Mesh::RecordUpload(ID3D12GraphicsCommandList* cmdList) {
	cmdList->CopyBufferRegion(m_vertexBuffer.Get(), 0, m_vbUploadBuffer.Get(), 0, m_vertexBufferView.SizeInBytes);
	cmdList->CopyBufferRegion(m_indexBuffer.Get(), 0, m_ibUploadBuffer.Get(), 0, m_indexBufferView.SizeInBytes);

	D3D12_RESOURCE_BARRIER barrier[2] = {};
	barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier[0].Transition.pResource = m_vertexBuffer.Get();
	barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	barrier[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier[1] = barrier[0];
	barrier[1].Transition.pResource = m_indexBuffer.Get();
	barrier[1].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	cmdList->ResourceBarrier(2, barrier);
}

void Mesh::ReleaseUploadBuffer() {
	m_vbUploadBuffer.Reset();
	m_ibUploadBuffer.Reset();
}

void Mesh::Bind(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmdList->IASetIndexBuffer(&m_indexBufferView);
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) {
	cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}