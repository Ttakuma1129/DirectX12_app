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
#include "Skybox.h"
#include "../Resources/FrameResources.h"
#include "../Resources/Mesh.h"
#include "../Resources/Texture.h"
#include "../Resources/ModelLoader.h"
#include "../Resources/SceneObject.h"

class  Scene;

class Renderer{
public:
	bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator);

	bool InitializeSkybox(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, const std::string faceFiles[6]);

	// メッシュを読み込みインデックスを返す
	int LoadMesh(ID3D12Device* device, const std::string& filepath);

	// テクスチャを読み込みインデックスを返す
	int LoadTexture(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, const std::string& filepath);

	void Render(
		ID3D12GraphicsCommandList* cmdList,
		const Scene& scene,
		uint32_t frameIndex,
		FrameResources& frame,
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
		uint32_t width,
		uint32_t height);

	// オブジェクト登録
	int RegisterObject(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, SceneObject& obj);

	DescriptorHeap& GetImGuiSrvHeap() {
		return m_imguiSrvHeap;
	}
	const std::vector<std::string>& getModelPaths() const {
		return m_modelPaths;
	}

	bool IsSkyboxEnabled() const {
		return m_skyboxEnabled;
	}

private:
	static constexpr uint32_t FRAME_COUNT = 2;
	static constexpr uint32_t MAX_OBJECTS = 16;
	static constexpr uint32_t MAX_TEXTURES = 32;

	RootSignature m_rootSignature; // ルートシグネチャ
	PipelineState m_pipelineState; // パイプラインステート
	DescriptorHeap m_srvHeap; // srv
	DescriptorHeap m_imguiSrvHeap; // ImGUI用ディスクリプタヒープ
	Texture m_texture;
	Skybox m_skybox;

	bool m_skyboxEnabled = false;
	
	// メッシュ管理
	std::vector<Mesh> m_meshes;
	std::vector<std::string> m_modelPaths;
	std::unordered_map<std::string, uint32_t> m_meshMap;

	// テクスチャ管理
	std::vector<Texture> m_textures;
	std::vector<std::string> m_texturePath;
	std::unordered_map<std::string, uint32_t> m_textureMap;
	uint32_t m_nextSrvSlot = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_objectCB[FRAME_COUNT][MAX_OBJECTS]; // オブジェクトごとの定数バッファ
	ObjectConstant* m_objectMapped[FRAME_COUNT][MAX_OBJECTS] = {}; // オブジェクトごとのマップ
};
