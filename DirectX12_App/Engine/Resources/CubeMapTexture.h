#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>

class CubeMapTexture {
public:
	
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;
};