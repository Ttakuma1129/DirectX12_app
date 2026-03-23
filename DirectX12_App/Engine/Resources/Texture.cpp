#include "Texture.h"

bool Texture::Create(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	uint32_t width,
	uint32_t height,
	const void* pixels,
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle) {

	HRESULT hr;

	// テクスチャリソースの設定
	D3D12_HEAP_PROPERTIES texHeapProps = {};
	texHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	
	D3D12_RESOURCE_DESC texResDesc = {};
	texResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texResDesc.Width = width;
	texResDesc.Height = height;
	texResDesc.DepthOrArraySize = 1;
	texResDesc.MipLevels = 1;
	texResDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texResDesc.SampleDesc.Count = 1;
	texResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// テクスチャリソースの作成
	hr = device->CreateCommittedResource(
		&texHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texResDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_texture));
	if (FAILED(hr)) {
		return false;
	}

	// アライメント情報の取得
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
	UINT64 totalBytes = 0;
	device->GetCopyableFootprints(
		&texResDesc,
		0,
		1,
		0,
		&footprint,
		nullptr,
		nullptr,
		&totalBytes);

	// アップロード用中間バッファの設定
	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC uploadResDesc = {};
	uploadResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadResDesc.Width = totalBytes;
	uploadResDesc.Height = 1;
	uploadResDesc.DepthOrArraySize = 1;
	uploadResDesc.MipLevels = 1;
	uploadResDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadResDesc.SampleDesc.Count = 1;
	uploadResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// アップロード用中間バッファの作成
	hr = device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadResDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_uploadBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// ピクセルデータを中間バッファに書き込み
	uint8_t* mapped = nullptr;
	hr = m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	if (FAILED(hr)) {
		return false;
	}

	uint32_t srcRowBytes = width * 4; // RGBA (4バイト/ピクセル)
	for (uint32_t y = 0; y < height; ++y) {
		memcpy(
			mapped + y * footprint.Footprint.RowPitch,
			static_cast<const uint8_t*>(pixels) + y * srcRowBytes,
			srcRowBytes);
	}
	m_uploadBuffer->Unmap(0, nullptr);

	// 中間バッファからテクスチャリソースへのコピーを記録
	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = m_texture.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = m_uploadBuffer.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint = footprint;

	cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

}