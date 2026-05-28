#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>
#include <vector>
#include <utility>

#include "DescriptorHeap.h"
#include "../Resources/FrameResources.h"
#include "CommandContext.h"

// ライブラリのリンクを指定
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

class GfxDevice {
public:
	~GfxDevice();

	// GPUリソースの作成
	bool Initialize(HWND hwnd, uint32_t width, uint32_t height);

	// フレーム制御関連の作成
	bool InitializeFrameResources();

	void BeginFrame();

	void WaitForGPU();

	void EndFrame();

	// 外部から取得するためのゲッター
	ID3D12Device* GetDevice() const {
		return m_device.Get();
	}

	ID3D12GraphicsCommandList* GetCommandList() {
		return m_commandContext.GetCommandList();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() {
		return m_rtvHeap.GetCPUHandle(m_frameIndex);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() {
		return m_dsvHeap.GetCPUHandle(0);
	}

	ID3D12CommandQueue* GetCommandQueue() const {
		return m_commandQueue.Get();
	}

	IDXGISwapChain4* GetSwapChain() const {
		return m_swapChain.Get();
	}

	FrameResources& GetCurrentFrame() {
		return m_frames[m_frameIndex];
	}

	uint32_t GetFrameIndex() const {
		return m_frameIndex;
	}

	uint32_t GetWidth() const {
		return m_width;
	}

	uint32_t GetHeight() const {
		return m_height;
	}

private:
	static constexpr uint32_t FRAME_COUNT = 2;
	// ComptrでReleaseを自動化
	Microsoft::WRL::ComPtr<IDXGIFactory7> m_dxgiFactory; // ファクトリ
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter; // アダプタ
	Microsoft::WRL::ComPtr<ID3D12Device> m_device; // デバイス
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue; // コマンドキュー
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain; // スワップチェーン
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence; // フェンス

	DescriptorHeap m_rtvHeap; // RTV
	DescriptorHeap m_dsvHeap; // DSV
	CommandContext m_commandContext; //コマンドリスト

	// リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> m_backBuffers[FRAME_COUNT]; // バックバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthBuffer; // 深度バッファ
	FrameResources m_frames[FRAME_COUNT]; // フレームリソース
	// このフレーム中にEnqueueされたフェンス値が決まっていないリソース
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_pendingThisFrame;
	// {fence値, リソース} フェンス値とペアの解放待機リソース
	std::vector<std::pair<uint64_t, Microsoft::WRL::ComPtr<ID3D12Resource>>> m_releaseQueue;

	UINT64 m_fenceValue = 0;
	HANDLE m_fenceEvent = nullptr;
	uint32_t m_frameIndex = 0;
	uint32_t m_width;
	uint32_t m_height;

};