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
	bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, const std::string faceFiles[6]);

	void Render(ID3D12GraphicsCommandList* cmdList, const SceneConstant* sceneConstant, D3D12_GPU_VIRTUAL_ADDRESS sceneCBAddress);
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	DescriptorHeap m_srvHeap;
	CubeMapTexture m_cubeMap;
	Mesh m_cubeMesh;
};