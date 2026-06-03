#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>

class CubeMapTexture {
public:
	bool Create(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const std::string faceFiles[6], D3D12_CPU_DESCRIPTOR_HANDLE srvHandle);

	void ReleaseUploadBuffer();

	ID3D12Resource* GetResource() const {
		return m_texture.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
};