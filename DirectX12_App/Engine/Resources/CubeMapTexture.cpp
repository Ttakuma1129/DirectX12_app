#include "CubeMapTexture.h"
#include "../ThirdParty/stb_image.h"

bool CubeMapTexture::Create(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::string faceFiles[6], D3D12_CPU_DESCRIPTOR_HANDLE srvHandle) {
	HRESULT hr;

	// 1枚読んでサイズを確認
	int width, height, channels;
	unsigned char* testPixels = stbi_load(faceFiles[0].c_str(), &width, &height, &channels, 4);
	if (!testPixels) {
		OutputDebugStringA(("Failed to load cubemap face:" + faceFiles[0] + "\n").c_str());
		return false;
	}
	stbi_image_free(testPixels);

	// テクスチャリソースの作成(6面分)
	D3D12_HEAP_PROPERTIES tesHeapProps = {};
	tesHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 6;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = device->CreateCommittedResource(
		&tesHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_texture));
	if (FAILED(hr)) {
		return false;
	}

	// アライメント情報の取得
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprints[6] = {};
	UINT64 rowSizes[6] = {};
	UINT numRows[6] = {};
	UINT64 totalBytes = 0;

	device->GetCopyableFootprints(&texDesc, 0, 6, 0, footprints, numRows, rowSizes, &totalBytes);

	// アップロード用中間バッファの設定
	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC uploadDesc = {};
	uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadDesc.Width = totalBytes;
	uploadDesc.Height = 1;
	uploadDesc.DepthOrArraySize = 1;
	uploadDesc.MipLevels = 1;
	uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadDesc.SampleDesc.Count = 1;
	uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// アップロード用中間バッファの作成
	hr = device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
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

	// 6面分の画像データを書き込み＆コピーコマンド記録
	for (uint32_t face = 0; face < 6; ++face) {
		int w, h, ch;
		unsigned char* pixels = stbi_load(faceFiles[face].c_str(), &w, &h, &ch, 4);
		if (!pixels) {
			OutputDebugStringA(("Failed to load cubemap face:" + faceFiles[face] + "\n").c_str());
			m_uploadBuffer->Unmap(0, nullptr);
			return false;
		}

		uint32_t srcRowBytes = w * 4;
		uint8_t* dest = mapped + footprints[face].Offset;

		for (uint32_t y = 0; y < static_cast<uint32_t>(h);++y) {
			memcpy(
				dest + y * footprints[face].Footprint.RowPitch,
				pixels + y * srcRowBytes,
				srcRowBytes);
		}
		stbi_image_free(pixels);

		// 中間バッファからテクスチャリソースへのコピーを記録
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_texture.Get();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = face;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_uploadBuffer.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprints[face];

		cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}
	m_uploadBuffer->Unmap(0, nullptr);

	// COPY_DEST → PIXEL_SHADER_RESOURCEへバリアを遷移
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_texture.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdList->ResourceBarrier(1, &barrier);

	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(
		m_texture.Get(),
		&srvDesc,
		srvHandle);

	return true;
}

void CubeMapTexture::ReleaseUploadBuffer() {
	m_uploadBuffer.Reset();
}