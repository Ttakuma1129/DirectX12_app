#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <chrono>

#include "DescriptorHeap.h"
#include "../Resources/FrameResources.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"

// ライブラリのリンクを指定
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class GfxDevice {
public:
	~GfxDevice();

	bool Initialize(HWND hwnd, uint32_t width, uint32_t height);

	bool InitializeFrameResources();

	void BeginFrame();
	
	void EndFrame();

	// 外部からデバイスを取得するためのゲッター
	ID3D12Device* GetDevice() const {
		return m_device.Get();
	}

	// 外部からコマンドキューを取得するためのゲッター
	ID3D12CommandQueue* GetCommandQueue() const {
		return m_commandQueue.Get();
	}
	// 外部からスワップチェーンを取得するためのゲッター
	IDXGISwapChain4* GetSwapChain() const {
		return m_swapChain.Get();
	}


private:
	static constexpr uint32_t FRAME_COUNT = 2;
	// ComptrでReleaseを自動化
	Microsoft::WRL::ComPtr<IDXGIFactory7> m_dxgiFactory; // ファクトリ
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter; // アダプター
	Microsoft::WRL::ComPtr<ID3D12Device> m_device; // デバイス
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue; // コマンドキュー
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain; // スワップチェーン
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence; // フェンス

	DescriptorHeap m_rtvHeap; // RTV
	DescriptorHeap m_dsvHeap; // DSV
	DescriptorHeap m_srvHeap; // SRV
	CommandContext m_commandContext; //コマンドリスト
	RootSignature m_rootSignature; // ルートシグネチャ
	PipelineState m_pipelineState; // パイプラインステート

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[FRAME_COUNT]; // バックバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer; // 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer; // インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer; // 深度バッファ
	FrameResources m_frames[FRAME_COUNT];

	UINT64 m_fenceValue = 0;
	HANDLE m_fenceEvent = nullptr;
	uint32_t m_frameIndex = 0;
	uint32_t m_width;
	uint32_t m_height;
	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存

	struct Vertex{
		float position[3];
		float uv[2];
	};
	
};