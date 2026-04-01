#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <d3dcompiler.h>
#include <vector>
#include <string>
#include <unordered_map>

#include "RootSignature.h"
#include "PipelineState.h"
#include "DescriptorHeap.h"
#include "../Resources/FrameResources.h"
#include "../Resources/Mesh.h"
#include "../Resources/Texture.h"
#include "../Resources/ModelLoader.h"

class  Scene;

class Renderer{
public:
	bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator);

	// メッシュを読み込みインデックスを返す
	int LoadMesh(ID3D12Device* device, const std::string& filepath);

	void Render(
		ID3D12GraphicsCommandList* cmdList,
		const Scene& scene,
		uint32_t frameIndex,
		FrameResources& frame,
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
		uint32_t width,
		uint32_t height);

	DescriptorHeap& GetImGuiSrvHeap() {
		return m_imguiSrvHeap;
	}

private:
	static constexpr uint32_t FRAME_COUNT = 2;
	static constexpr uint32_t MAX_OBJECTS = 16;

	RootSignature m_rootSignature; // ルートシグネチャ
	PipelineState m_pipelineState; // パイプラインステート
	DescriptorHeap m_srvHeap; // srv
	DescriptorHeap m_imguiSrvHeap; // ImGUI用ディスクリプタヒープ
	Texture m_texture;
	Mesh m_cubeMesh; // メッシュ

	Microsoft::WRL::ComPtr<ID3D12Resource> m_objectCB[FRAME_COUNT][MAX_OBJECTS]; // オブジェクトごとの定数バッファ
	ObjectConstant* m_objectMapped[FRAME_COUNT][MAX_OBJECTS] = {}; // オブジェクトごとのマップ

	struct Vertex {
		float position[3];
		float uv[2];
	};
};
