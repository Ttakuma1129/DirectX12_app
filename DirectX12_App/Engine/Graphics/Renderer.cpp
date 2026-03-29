#include "Renderer.h"
#include "../../App/Scene.h"
#include "CommandContext.h"

#include "../ThirdParty/stb_image.h"

bool Renderer::Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator) {
	HRESULT hr;

	// RootSignatureの初期化
	if (!m_rootSignature.Initialize(device)) {
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
		device,
		m_rootSignature.GetRootSignature(),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		psBlob->GetBufferPointer(), psBlob->GetBufferSize())) {
		return false;
	}

	// OBJモデル読み込み
	ModelData modelData;
	if (!ModelLoader::LoadOBJ("App/ModelModels/uploads_files_2787791_Mercedes+Benz+GLS+580.obj", modelData)) {
		OutputDebugStringA("Failed to load OBJ model\n");
		return false;
	}

	//頂点バッファ・インデックスバッファの作成
	if (!m_cubeMesh.Create(
		device,
		modelData.vertices.data(),
		static_cast<uint32_t>(modelData.vertices.size() * sizeof(ModelVertex)),
		sizeof(ModelVertex),
		modelData.indices.data(),
		static_cast<uint32_t>(modelData.indices.size()))) {
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

			hr = device->CreateCommittedResource(
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

	// SRV用ディスクリプタヒープの作成
	if (!m_srvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true)) {
		return false;
	}

	// ImGui用SRV用ディスクリプタヒープの作成
	if (!m_imguiSrvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true)) {
		return false;
	}

	// 画像読み込み
	int texWidth, texHeight, channels;
	unsigned char* pixels = stbi_load(
		"App/Textures/sample.png",	// 画像ファイルのパス(実行ファイルからのパス)
		&texWidth, &texHeight, &channels, 4);

	if (!pixels) {
		return false;
	}
	
	CommandContext uploadContext;
	uploadContext.Initialize(device, allocator);
	uploadContext.Begin(allocator);

	// テクスチャの作成
	m_texture.Create(device, uploadContext.GetCommandList(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), pixels, m_srvHeap.GetCPUHandle(0));

	// コマンド実行・GPUの完了待ち
	uploadContext.End();
	uploadContext.Execute(commandQueue);

	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		return true;
	}

	HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	commandQueue->Signal(fence.Get(), 1);
	fence->SetEventOnCompletion(1, event);
	WaitForSingleObject(event, INFINITE);
	CloseHandle(event);

	// GPUの完了後に中間バッファを解放
	stbi_image_free(pixels);
	m_texture.ReleaseUploadBuffer();

	return true;
}

void Renderer::Render(
	ID3D12GraphicsCommandList* cmdList,
	const Scene& scene,
	uint32_t frameIndex,
	FrameResources& frame,
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
	uint32_t width,
	uint32_t height) {

	using namespace DirectX;

	// パイプライン設定
	cmdList->SetGraphicsRootSignature(m_rootSignature.GetRootSignature());
	cmdList->SetPipelineState(m_pipelineState.GetPipelineState());

	// SRVヒープをセット
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.GetHeap() };
	cmdList->SetDescriptorHeaps(1, heaps);

	// ルートパラメータ2にSRVテーブルをバインド
	cmdList->SetGraphicsRootDescriptorTable(2, m_srvHeap.GetGPUHandle(0));

	// 定数バッファを書き込み
	SceneConstant* mapped = frame.GetConstantMapped();
	XMStoreFloat4x4(&mapped->view, XMMatrixTranspose(scene.GetViewMatrix()));
	XMStoreFloat4x4(&mapped->proj, XMMatrixTranspose(scene.GetProjMatrix()));

	// 定数バッファをバインド
	cmdList->SetGraphicsRootConstantBufferView(0 ,frame.GetConstantBuffer()->GetGPUVirtualAddress());

	// Viewportを設定
	D3D12_VIEWPORT viewport = {
		0.0f,
		0.0f,
		static_cast<float>(width),
		static_cast<float>(height),
		0.0f,
		1.0f,
	};
	cmdList->RSSetViewports(1, &viewport);

	// ScissorRectを設定
	D3D12_RECT scissorRect = {
		0,
		0,
		static_cast<LONG>(width),
		static_cast<LONG>(height)
	};
	cmdList->RSSetScissorRects(1, &scissorRect);

	// RenderTargetを設定
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// PrimitiveTopologyを設定
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VertexBufferとIndexBufferの場所を設定
	m_cubeMesh.Bind(cmdList);

	// 描画ループ
	for (uint32_t o = 0; o < OBJECT_COUNT; ++o) {
		// オブジェクトごとのModel行列を計算
		XMMATRIX model = scene.GetModelMatrix(o);


		// 定数バッファに書き込み
		XMStoreFloat4x4(&m_objectMapped[frameIndex][o]->model, XMMatrixTranspose(model));

		// オブジェクト定数をバインド
		cmdList->SetGraphicsRootConstantBufferView(1, m_objectCB[frameIndex][o]->GetGPUVirtualAddress());

		// 描画
		m_cubeMesh.Draw(cmdList);
	}
}