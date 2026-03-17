#include "GfxDevice.h"
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

	// ディスクリプタヒープの作成
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

	// RootSignatureの初期化
	if (!m_rootSignature.Initialize(m_device.Get())) {
		return false;
	}

	// シェーダーコンパイル
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBlob, psBlob, errorBlob;

	hr = D3DCompileFromFile(
		L"Shaders/VertexShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errorBlob
	);
	if (FAILED(hr)) {
		// ファイルが見つからない場合はerrorBlobもnull
		if (errorBlob) {
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}
		OutputDebugStringA("VS compile failed\n");
		return false;
	}

	hr = D3DCompileFromFile(
		L"Shaders/PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob
	);
	if (FAILED(hr)) {
		// ファイルが見つからない場合はerrorBlobもnull
		if (errorBlob) {
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}
		OutputDebugStringA("VS compile failed\n");
		return false;
	}

	// パイプラインステートオブジェクトの作成
	if (!m_pipelineState.Initialize(
		m_device.Get(),
		m_rootSignature.GetRootSignature(),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		psBlob->GetBufferPointer(), psBlob->GetBufferSize())) {
		return false;
	}

	// 頂点バッファの作成
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = sizeof(Vertex) * 3;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	if (FAILED(hr)) {
		return false;
	}

	// 頂点バッファをGPUに転送(マッピング)
	Vertex vertices[] = {
		{ {0.0f, 0.75f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },	// 上・赤
		{ {0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },	// 右下・緑
		{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },	// 左下・青
	};

	void* mapped = nullptr;
	hr = m_vertexBuffer->Map(0, nullptr, &mapped);
	if (FAILED(hr)) {
		return false;
	}

	memcpy(mapped, vertices, sizeof(vertices));
	m_vertexBuffer->Unmap(0, nullptr);

	// 頂点バッファビューの作成
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(Vertex) * 3; // バッファ全体のサイズ
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);	// 1頂点のバッファサイズ

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
	
	// パイプライン設定
	cmdList->SetGraphicsRootSignature(m_rootSignature.GetRootSignature());
	cmdList->SetPipelineState(m_pipelineState.GetPipelineState());

	// 行列の計算
	using namespace DirectX;

	// Model行列
	XMMATRIX model = XMMatrixIdentity();

	// View行列 (カメラの設定)
	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f); // カメラ位置
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // 注視点
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向
	XMMATRIX view = XMMatrixLookAtLH(eye, target, up); // View行列

	// Projection行列　(透視投影)
	float fov = XMConvertToRadians(60.0f); // 視野角
	float aspect = static_cast<float>(m_width) / m_height;
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 100.0f);

	// MVP行列 (Model x View x Projection)
	XMMATRIX mvp = model * view * proj;

	// 定数バッファに書き込み
	SceneConstant* mapped = m_frames[m_frameIndex].GetConstantMapped();
	XMStoreFloat4x4(&mapped->mvp, XMMatrixTranspose(mvp));

	// 定数バッファをGPUにセット
	cmdList->SetGraphicsRootConstantBufferView(
		0,
		m_frames[m_frameIndex].GetConstantBuffer()->GetGPUVirtualAddress()
	);

	// Viewportを設定
	D3D12_VIEWPORT viewport = {
		0.0f,
		0.0f,
		static_cast<float>(m_width),
		static_cast<float>(m_height),
		0.0f,
		1.0f,
	};
	cmdList->RSSetViewports(1, &viewport);

	// ScissorRectを設定
	D3D12_RECT scissorRect = {
		0,
		0,
		static_cast<LONG>(m_width),
		static_cast<LONG>(m_height)
	};
	cmdList->RSSetScissorRects(1, &scissorRect);

	// RenderTargetを設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap.GetCPUHandle(m_frameIndex);
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// PrimitiveTopologyを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VertexBufferの場所を設定
	cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

	// 描画
	cmdList->DrawInstanced(3, 1, 0, 0);

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