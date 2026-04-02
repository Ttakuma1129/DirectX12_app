#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

#include "DescriptorHeap.h"
#include "../Resources//Mesh.h"
#include "../Resources/CubeMapTexture.h"
#include "../Resources/FrameResources.h"

class Skybox {
public:

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	DescriptorHeap m_srvHeap;
	CubeMapTexture m_cubeMap;
	Mesh m_cubeMesh;
};