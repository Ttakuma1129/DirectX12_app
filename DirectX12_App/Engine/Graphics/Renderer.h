#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include "../Resources/FrameResources.h"
#include "../Resources/Mesh.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "DescriptorHeap.h"

class  Scene;

class Renderer{
public:
	bool Initialize(ID3D12Device* device);

	void Render(
		ID3D12GraphicsCommandList* cmdList,
		const Scene& scene,
		uint32_t frameIndex,
		FrameResources& frame,
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
		uint32_t width,
		uint32_t height);

private:
	static constexpr uint32_t FRAME_COUNT = 2;
	static constexpr uint32_t OBJECT_COUNT = 2;

	RootSignature m_rootSignature; // ルートシグネチャ
	PipelineState m_pipelineState; // パイプラインステート
	Mesh m_cubeMesh; // メッシュ

	Microsoft::WRL::ComPtr<ID3D12Resource> m_objectCB[FRAME_COUNT][OBJECT_COUNT]; // オブジェクトごとの定数バッファ
	ObjectConstant* m_objectMapped[FRAME_COUNT][OBJECT_COUNT] = {}; // オブジェクトごとのマップ
};
