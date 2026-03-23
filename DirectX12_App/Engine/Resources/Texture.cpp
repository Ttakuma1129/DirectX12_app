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



}