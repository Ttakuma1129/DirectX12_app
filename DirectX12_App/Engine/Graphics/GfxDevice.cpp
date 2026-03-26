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

	// SRV用ディスクリプタヒープの作成
	if(!m_srvHeap.Initialize(m_device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true)){
		return false;
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
		OutputDebugStringA("PS compile failed\n");
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

	// 立方体の頂点データ
	Vertex vertices[] = {
		// 面0: 手前の面 (頂点0～3)
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f} }, // 左上
		{ { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f} }, // 右上
		{ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f} }, // 左下
		{ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f} }, // 右下

		// 面1: 向かって左の面 (頂点4～7)
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} }, // 左上
		{ {-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f} }, // 右上
		{ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f} }, // 左下
		{ {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f} }, // 右下

		// 面2: 向かって右の面 (頂点8～11)
		{ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f} }, // 左上
		{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f} }, // 右上
		{ { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f} }, // 左下
		{ { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f} }, // 右下

		// 面3: 奥の面 (頂点12～15)
		{ {-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f} },	// 左上
		{ { 0.5f,  0.5f, 0.5f}, {1.0f, 0.0f} },	// 右上
		{ {-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f} },	// 左下
		{ { 0.5f, -0.5f, 0.5f}, {1.0f, 1.0f} },	// 右下

		// 面4: 下の面 (頂点16～19)		
		{ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f} }, // 左上
		{ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f} }, // 右上	
		{ {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f} }, // 左下
		{ { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f} }, // 右下

		// 面5: 上の面 (頂点20～23)
		{ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f} }, // 左上
		{ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f} }, // 右上
		{ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f} }, // 左上
		{ { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f} }, // 右下

	};

	// インデックスデータ
	uint16_t indices[] = {
		// 手前 (z = -0.5)
		0, 1, 2,
		2, 1, 3,
		// 左 (z = 0.5)
		4, 5, 6,
		6, 5, 7,
		// 右 (x = -0.5)
		8, 9, 10,
		10, 9, 11,
		// 奥 (x = 0.5)
		13, 12, 15,
		15, 12, 14,
		// 下 (y = 0.5)
		16, 17, 18,
		18, 17, 19,
		// 上 (y = -0.5)
		20, 21, 22,
		22, 21, 23,
	};

	// 頂点バッファとインデックスバッファの作成
	if (!m_cubeMesh.Create(m_device.Get(), vertices, sizeof(vertices), sizeof(Vertex), indices, 36)) {
		return false;
	}

	// 定数バッファ作成をフレーム*オブジェクト分ループする
	for (uint32_t f = 0; f < FRAME_COUNT; ++f) {
		for (uint32_t o = 0; o < OBJECT_COUNT; ++o) {
			D3D12_HEAP_PROPERTIES cbHeapProps = {};
			cbHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

			D3D12_RESOURCE_DESC cbResDesc = {};
			cbResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			cbResDesc.Width = sizeof(ObjectConstant);
			cbResDesc.Height = 1;
			cbResDesc.DepthOrArraySize = 1;
			cbResDesc.MipLevels = 1;
			cbResDesc.Format = DXGI_FORMAT_UNKNOWN;
			cbResDesc.SampleDesc.Count = 1;
			cbResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			hr = m_device->CreateCommittedResource(
				&cbHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&cbResDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_objectCB[f][o]));
			if (FAILED(hr)) {
				return false;
			}

			hr = m_objectCB[f][o]->Map(0, nullptr, reinterpret_cast<void**>(&m_objectMapped[f][o]));
			if (FAILED(hr)) {
				return false;
			}
		}
	}

	// 画像読み込み
	int texWidth, texHeight, channels;
	unsigned char* pixels = stbi_load(
		"App/Textures/sample.png",	// 画像ファイルのパス(実行ファイルからのパス)
		&texWidth, &texHeight, &channels, 4);

	if (!pixels) {
		return false;
	}

	m_commandContext.Begin(m_frames[0].GetAllocator());

	// テクスチャの作成
	m_texture.Create(m_device.Get(), m_commandContext.GetCommandList(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), pixels, m_srvHeap.GetCPUHandle(0));

	// コマンド実行・GPUの完了待ち
	m_commandContext.End();
	m_commandContext.Execute(m_commandQueue.Get());

	m_fenceValue++;
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
	m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	WaitForSingleObject(m_fenceEvent, INFINITE);

	// GPUの完了後に中間バッファを解放
	stbi_image_free(pixels);
	m_texture.ReleaseUploadBuffer();

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

	// パイプライン設定
	cmdList->SetGraphicsRootSignature(m_rootSignature.GetRootSignature());
	cmdList->SetPipelineState(m_pipelineState.GetPipelineState());

	// SRVヒープをセット
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.GetHeap() };
	cmdList->SetDescriptorHeaps(1, heaps);

	// ルートパラメータ2にSRVテーブルをバインド
	cmdList->SetGraphicsRootDescriptorTable(2, m_srvHeap.GetGPUHandle(0));

	// 行列の計算
	using namespace DirectX;

	// View行列 (カメラの設定)
	XMVECTOR eye = XMVectorSet(0.0f, 0.7f, -3.0f, 0.0f); // カメラ位置
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // 注視点
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向
	XMMATRIX view = XMMatrixLookAtLH(eye, target, up); // View行列

	// Projection行列　(透視投影)
	float fov = XMConvertToRadians(45.0f); // 視野角
	float aspect = static_cast<float>(m_width) / m_height;
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 100.0f);

	// 定数バッファを書き込み
	SceneConstant* mapped = m_frames[m_frameIndex].GetConstantMapped();
	XMStoreFloat4x4(&mapped->view, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mapped->proj, XMMatrixTranspose(proj));

	// 定数バッファをバインド
	cmdList->SetGraphicsRootConstantBufferView(
		0,
		m_frames[m_frameIndex].GetConstantBuffer()->GetGPUVirtualAddress());

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
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// PrimitiveTopologyを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VertexBufferとIndexBufferの場所を設定
	m_cubeMesh.Bind(cmdList);

	// 描画ループ
	for (uint32_t o = 0; o < OBJECT_COUNT; ++o) {
		// オブジェクトごとのModel行列を計算
		XMMATRIX model;
		if (o == 0) {
			model = XMMatrixRotationY(elapsed * XM_2PI * 0.5f) * XMMatrixTranslation(-1.5f, 0.0f, 0.0f);
			
		}
		else {
			model = XMMatrixRotationY(-elapsed * XM_2PI * 0.5f) * XMMatrixTranslation(1.5f, 0.0f, 0.0f);

		}

		// 定数バッファに書き込み
		XMStoreFloat4x4(&m_objectMapped[m_frameIndex][o]->model, XMMatrixTranspose(model));
		
		// オブジェクト定数をバインド
		cmdList->SetGraphicsRootConstantBufferView(1, m_objectCB[m_frameIndex][o]->GetGPUVirtualAddress());
	
		// 描画
		m_cubeMesh.Draw(cmdList);
	}
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