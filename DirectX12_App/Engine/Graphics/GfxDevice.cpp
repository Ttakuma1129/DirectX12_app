#include "GfxDevice.h"
#include "../ThirdParty/stb_image.h"
#include <iostream>

GfxDevice::~GfxDevice() {
	// フレームのGPU処理完了を待つ
	if (m_fence && m_fenceEvent) {
		if (m_fence->GetCompletedValue() < m_fenceValue) {
			m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}
	if (m_fenceEvent) {
		CloseHandle(m_fenceEvent);
	}
}

bool GfxDevice::Initialize(HWND hwnd, uint32_t width, uint32_t height) {

	UINT createFactoryFlags = 0;
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	}
#endif // _DEBUG

	HRESULT hr;
	m_width = width;
	m_height = height;

	// DXGIファクトリーの作成
	hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
	if (FAILED(hr)) {
		return false;
	}

	// デバイスの作成
	// 一番性能の良いGPUを選択
	for (UINT i = 0; m_dxgiFactory->EnumAdapterByGpuPreference(
		i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&m_adapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
		hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
		if (SUCCEEDED(hr))
			break;
	}
	
	if (FAILED(hr)) {
		return false;
	}

	// コマンドキューの作成
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if(FAILED(hr)){
		return false;
	}

	// スワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画の出力先
	swapChainDesc.BufferCount = FRAME_COUNT; // 表用と裏用の2つ
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH; 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	hr = m_dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(), // 描画完了を待つためにコマンドキューを渡す
		hwnd, // 描画先のウィンドウハンドル
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	);
	if (FAILED(hr)) {
		return false;
	}
	// IDXGISwapChain4にキャストして保存
	swapChain.As(&m_swapChain);

	// RTV用ディスクリプタヒープの作成
	if (!m_rtvHeap.Initialize(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FRAME_COUNT, false)) {
		return false;
	}
	for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
		if (FAILED(hr)) {
			return false;
		}

		m_device->CreateRenderTargetView(
			m_backBuffers[i].Get(),
			nullptr,
			m_rtvHeap.GetCPUHandle(i)
		);
	}

	// DSV用ディスクリプタヒープの作成
	if (!m_dsvHeap.Initialize(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false)) {
		return false;
	}

	// 深度バッファの設定
	D3D12_HEAP_PROPERTIES depthHeapProps = {};
	depthHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = width;
	depthResDesc.Height = height;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.MipLevels = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// クリア値設定
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// 深度バッファの作成
	hr = m_device->CreateCommittedResource(
		&depthHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthBuffer));
	if (FAILED(hr)) {
		return false;
	}

	//DSVの作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	m_device->CreateDepthStencilView(
		m_depthBuffer.Get(),
		&dsvDesc,
		m_dsvHeap.GetCPUHandle(0));

	return true;
}

bool GfxDevice::InitializeFrameResources() {
	HRESULT hr;

	// FrameResourcesの初期化
	for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
		if (!m_frames[i].Initialize(m_device.Get())) {
			return false;
		}
	}

	// Fenceの作成
	hr = m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	if (FAILED(hr)) {
		return false;
	}

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr) {
		return false;
	}

	// CommandContextの初期化
	if (!m_commandContext.Initialize(m_device.Get(), m_frames[0].GetAllocator())) {
		return false;
	}

	return true;
}

void GfxDevice::BeginFrame() {
	// 現在のフレームインデックス取得
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// 前フレームのGPU完了を待つ
	uint64_t completedValue = m_fence->GetCompletedValue();
	if (completedValue < m_frames[m_frameIndex].fenceValue) {
		m_fence->SetEventOnCompletion(m_frames[m_frameIndex].fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// コマンド記録開始
	m_commandContext.Begin(m_frames[m_frameIndex].GetAllocator());

	// PRESENTからRENDER_TARGETへバリアを変更
	m_commandContext.TransitionBarrier(m_backBuffers[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// レンダーターゲットをクリア
	const float clearColor[] = { 0.0f, 0.2f,0.4f,1.0f };
	m_commandContext.ClearRenderTarget(m_rtvHeap.GetCPUHandle(m_frameIndex), clearColor);

	// コマンドリストを取得
	auto* cmdList = m_commandContext.GetCommandList();
	
	// 深度バッファをクリア
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap.GetCPUHandle(0);
	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void GfxDevice::EndFrame() {
	// RENDER_TARGETからPRESENTへバリアを変更
	m_commandContext.TransitionBarrier(m_backBuffers[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	// コマンド記録終了
	m_commandContext.End();

	// GPUへコマンドを送信
	m_commandContext.Execute(m_commandQueue.Get());

	// バックバッファを表示する
	m_swapChain->Present(1, 0);

	// GPUの処理が終わったらm_fenceValueを増やす
	m_frames[m_frameIndex].fenceValue = ++m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
}