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

class Scene;
class Chunk;
class World;

class Renderer{
public:
	bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator);

	bool InitializeSkybox(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, const std::string faceFiles[6]);

	bool InitializeShadow(ID3D12Device* device);

	// メッシュを読み込みインデックスを返す
	int LoadMesh(ID3D12Device* device, const std::string& filepath);

	// テクスチャを読み込みインデックスを返す
	int LoadTexture(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const std::string& filepath);

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
	int RegisterObject(ID3D12Device* device, ID3D12CommandQueue* commandQueue, SceneObject& obj);

	int RegisterChunk(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const Chunk& chunk, const World& world, int chunkX, int chunkZ, const std::string& texturePath, SceneObject& obj);

	DescriptorHeap& GetImGuiSrvHeap() {
		return m_imguiSrvHeap;
	}
	const std::vector<std::string>& GetModelPaths() const {
		return m_modelPaths;
	}
	const std::vector<std::string>& GetTexturePaths() const {
		return m_texturePath;
	}

	bool IsSkyboxEnabled() const {
		return m_skyboxEnabled;
	}

private:
	static constexpr uint32_t FRAME_COUNT = 2;
	static constexpr uint32_t MAX_OBJECTS = 64;
	static constexpr uint32_t MAX_TEXTURES = 32;
	static constexpr uint32_t SHADOW_MAP_SIZE = 2048;
	static constexpr uint32_t SHADOW_SRV_SLOT = MAX_TEXTURES - 1;

	RootSignature m_rootSignature; // ルートシグネチャ
	PipelineState m_pipelineState; // パイプラインステート
	DescriptorHeap m_srvHeap; // srv
	DescriptorHeap m_imguiSrvHeap; // ImGUI用ディスクリプタヒープ
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
	uint32_t m_srvSlot = 0;

	// シャドウマップ管理
	Microsoft::WRL::ComPtr<ID3D12Resource> m_shadowMap; // 深度テクスチャ
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_shadowPSO; // 深度のみ・PSなし
	DescriptorHeap m_shadowDsvHeap;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_objectCB[FRAME_COUNT][MAX_OBJECTS]; // オブジェクトごとの定数バッファ
	ObjectConstant* m_objectMapped[FRAME_COUNT][MAX_OBJECTS] = {}; // オブジェクトごとのマップ
};
