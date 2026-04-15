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

	// 定数バッファ作成をMAX_OBJECTS分ループする
	for (uint32_t f = 0; f < FRAME_COUNT; ++f) {
		for (uint32_t o = 0; o < MAX_OBJECTS; ++o) {
			D3D12_HEAP_PROPERTIES cbHeapProps = {};
			cbHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

			D3D12_RESOURCE_DESC cbResDesc = {};
			cbResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			cbResDesc.Width = (sizeof(ObjectConstant) + 255) & ~255;
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
	if (!m_srvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MAX_TEXTURES, true)) {
		return false;
	}

	// ImGui用SRV用ディスクリプタヒープの作成
	if (!m_imguiSrvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true)) {
		return false;
	}

	return true;
}

bool Renderer::InitializeSkybox(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, const std::string faceFiles[6]) {
	// スカイボックスキューブ作成
	if (!m_skybox.Initialize(device, commandQueue, allocator, faceFiles)) {
		OutputDebugStringA("Skybox Initialization failed\n");
		return false;
	}
	m_skyboxEnabled = true;
	return true;
}

bool Renderer::InitializeShadow(ID3D12Device* device) {
	HRESULT hr;

	// シャドウマップディスクリプタヒープの設定
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = SHADOW_MAP_SIZE;
	texDesc.Height = SHADOW_MAP_SIZE;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// クリア値の設定
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;

	// シャドウマップテクスチャ作成
	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_shadowMap));
	if (FAILED(hr)) {
		return false;
	}

	// 書き込み用DSV
	if (!m_shadowDsvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false)) {
		return false;
	}
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(m_shadowMap.Get(), &dsvDesc, m_shadowDsvHeap.GetCPUHandle(0));
	
	// シェーダーから読む用SRV
	if (!m_shadowSrvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true)) {
		return false;
	}
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_shadowMap.Get(), &srvDesc, m_shadowSrvHeap.GetCPUHandle(0));
	
	// シャドウパス用PSO
	Microsoft::WRL::ComPtr<ID3D10Blob> shadowVS, errorBlob;
	hr = D3DCompileFromFile(
		L"Shaders/ShadowVS.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shadowVS,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}
		return false;
	}

	// 頂点入力レイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			24,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		}
	};

	// パイプラインステートオブジェクト設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.GetRootSignature();
	psoDesc.VS = { shadowVS->GetBufferPointer(), shadowVS->GetBufferSize() };
	psoDesc.PS = { nullptr, 0 };
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.DepthBias = 1000;
	psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;

	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.NumRenderTargets = 0;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = UINT_MAX;

	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_shadowPSO));
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

int Renderer::LoadMesh(ID3D12Device* device, const std::string& filepath) {
	// 読み込み済みならインデックスを返す
	auto it = m_meshMap.find(filepath);
	if (it != m_meshMap.end()) {
		return it->second;
	}

	// OBJ読み込み
	ModelData modelData;
	if (!ModelLoader::LoadOBJ(filepath, modelData)) {
		return -1;
	}

	// メッシュ作成
	Mesh mesh;
	if (!mesh.Create(
		device,
		modelData.vertices.data(),
		static_cast<uint32_t>(modelData.vertices.size() * sizeof(ModelVertex)),
		sizeof(ModelVertex),
		modelData.indices.data(),
		static_cast<uint32_t>(modelData.indices.size()))) {
		return -1;

	}

	uint32_t index = static_cast<uint32_t>(m_meshes.size());
	m_meshes.push_back(std::move(mesh));
	m_modelPaths.push_back(filepath);
	m_meshMap[filepath] = index;
	return index;
}

int Renderer::LoadTexture(ID3D12Device* device, ID3D12CommandQueue* commandQueue, const std::string& filepath) {
	// 読み込み済みならインデックスを返す
	auto it = m_textureMap.find(filepath);
	if (it != m_textureMap.end()) {
		return it->second;
	}

	// スロット上限かチェック
	if (m_srvSlot >= MAX_TEXTURES) {
		OutputDebugStringA("Texture slot limit reached\n");
		return -1;
	}

	// 画像読み込み
	int texWidth, texHeight, channels;
	unsigned char* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &channels, 4);
	if (!pixels) {
		OutputDebugStringA(("Failed to laod texture:" + filepath + "\n").c_str());
		return -1;
	}

	// テクスチャ用のコマンドアロケータを作成
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return -1;
	}

	// アップロード用の中間バッファ
	CommandContext uploadContext;
	uploadContext.Initialize(device, allocator.Get());
	uploadContext.Begin(allocator.Get());

	// テクスチャ作成
	Texture texture;
	texture.Create(
		device,
		uploadContext.GetCommandList(),
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight),
		pixels,
		m_srvHeap.GetCPUHandle(m_srvSlot));

	uploadContext.End();
	uploadContext.Execute(commandQueue);

	// GPU完了待ち
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return -1;
	}

	// イベント作成
	HANDLE event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	commandQueue->Signal(fence.Get(), 1);
	fence->SetEventOnCompletion(1, event);
	WaitForSingleObject(event, INFINITE);
	CloseHandle(event);

	// 中間バッファを解放
	stbi_image_free(pixels);
	texture.ReleaseUploadBuffer();

	// テクスチャ登録
	uint32_t index = m_srvSlot;
	m_textures.push_back(std::move(texture));
	m_texturePath.push_back(filepath);
	m_textureMap[filepath] = index;
	m_srvSlot++;

	return static_cast<int>(index);
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

	const auto& objects = scene.GetObjects();
	uint32_t count = min(static_cast<uint32_t>(objects.size()), MAX_OBJECTS);

	// ライトのVP行列を計算
	const float* dir = scene.GetLightDir();
	XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(dir[0], dir[1], dir[2], 0.0f));
	XMVECTOR lightPos = lightDir * 20.0f;
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// ライトが真上、真下を向いているときのup補正
	if (fabsf(XMVectorGetY(lightDir)) > 0.99f) {
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	}

	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, XMVectorZero(), up);
	XMMATRIX lightProj = XMMatrixOrthographicLH(20.0f, 20.0f, 0.1f, 50.0f);
	XMMATRIX lightVP = lightView * lightProj;	

	// ルートパラメータ2にSRVテーブルをバインド
	cmdList->SetGraphicsRootDescriptorTable(2, m_srvHeap.GetGPUHandle(0));

	// 定数バッファを書き込み
	SceneConstant* mapped = frame.GetConstantMapped();
	const Camera& camera = scene.GetCamera();
	XMStoreFloat4x4(&mapped->view, XMMatrixTranspose(camera.GetViewMatrix()));
	XMStoreFloat4x4(&mapped->proj, XMMatrixTranspose(camera.GetProjMatrix()));

	// カメラ位置を書き込み
	XMFLOAT3 camPos = camera.GetPosition();
	mapped->cameraPos = { camPos.x, camPos.y, camPos.z, 1.0f };

	// ライト情報を書き込み
	mapped->lightDir = { dir[0], dir[1], dir[2], 0.0f };

	const float* color = scene.GetLightColor();
	mapped->lightColor = { color[0], color[1], color[2], 1.0f };

	const float* ambient = scene.GetAmbientColor();
	mapped->ambientColor = { ambient[0], ambient[1], ambient[2], 1.0f };

	mapped->specularParams = { scene.GetSapcIntensity(),scene.GetSpecShiciness(),0.0f,0.0f };
	XMStoreFloat4x4(&mapped->lightViewProj, XMMatrixTranspose(lightVP));
	mapped->shadowParams = { scene.GetShadowBias(),static_cast<float>(SHADOW_MAP_SIZE),0.0f,0.0f };
	
	// オブジェクトごとのModel行列を書き込む
	for (uint32_t o = 0; o < count; ++o) {
		if (objects[o].meshIndex >= m_meshes.size()) {
			continue;
		}
		XMMATRIX model = scene.GetModelMatrix(o);
		XMStoreFloat4x4(&m_objectMapped[frameIndex][o]->model, XMMatrixTranspose(model));
	}

	// 影の描画の設定
	cmdList->SetGraphicsRootSignature(m_rootSignature.GetRootSignature());
	cmdList->SetPipelineState(m_shadowPSO.Get());

	// 定数バッファをバインド
	cmdList->SetGraphicsRootConstantBufferView(0 ,frame.GetConstantBuffer()->GetGPUVirtualAddress());

	// Viewportを設定
	D3D12_VIEWPORT shadowViewport = {
		0.0f,
		0.0f,
		(float)SHADOW_MAP_SIZE,
		(float)SHADOW_MAP_SIZE,
		0.0f,
		1.0f,
	};
	cmdList->RSSetViewports(1, &shadowViewport);
	// ScissorRectを設定
	D3D12_RECT ShadowScissorRect = {
		0,
		0,
		(LONG)SHADOW_MAP_SIZE,
		(LONG)SHADOW_MAP_SIZE
	};
	cmdList->RSSetScissorRects(1, &ShadowScissorRect);
	
	D3D12_CPU_DESCRIPTOR_HANDLE shadowDsv = m_shadowDsvHeap.GetCPUHandle(0);
	cmdList->ClearDepthStencilView(shadowDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	cmdList->OMSetRenderTargets(0, nullptr, FALSE, &shadowDsv);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 影の描画
	for (uint32_t o = 0; o < count; ++o) {
		const auto& obj = objects[o];
		if (obj.meshIndex >= m_meshes.size()) {
			continue;
		}

		// オブジェクト定数をバインド
		cmdList->SetGraphicsRootConstantBufferView(1, m_objectCB[frameIndex][o]->GetGPUVirtualAddress());

		// メッシュをバインドして描画
		m_meshes[obj.meshIndex].Bind(cmdList);
		m_meshes[obj.meshIndex].Draw(cmdList);
	}

	// バリア
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_shadowMap.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmdList->ResourceBarrier(1, &barrier);

	// メイン描画の設定
	cmdList->SetPipelineState(m_pipelineState.GetPipelineState());

	// SRVヒープをセット
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.GetHeap() };
	cmdList->SetDescriptorHeaps(1, heaps);

	cmdList->SetGraphicsRootConstantBufferView(0, frame.GetConstantBuffer()->GetGPUVirtualAddress());

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

	// スカイボックス描画
	if (m_skyboxEnabled) {
		m_skybox.Render(cmdList, mapped, frame.GetConstantBuffer()->GetGPUVirtualAddress());
		
		// スカイボックス描画後、パイプラインを元に戻す
		cmdList->SetGraphicsRootSignature(m_rootSignature.GetRootSignature());
		cmdList->SetPipelineState(m_pipelineState.GetPipelineState());

		ID3D12DescriptorHeap* heaps[] = { m_srvHeap.GetHeap() };
		cmdList->SetDescriptorHeaps(1, heaps);
		cmdList->SetGraphicsRootDescriptorTable(2, m_srvHeap.GetGPUHandle(0));
	}

	// 描画オブジェクト数
	const auto& objects = scene.GetObjects();
	uint32_t count = min(static_cast<uint32_t>(objects.size()), MAX_OBJECTS);
	// 描画ループ
	for (uint32_t o = 0; o < count; ++o) {
		const auto& obj = objects[o];

		// メッシュが存在するかを確認
		if (obj.meshIndex >= m_meshes.size()) {
			continue;
		}

		// オブジェクトごとのModel行列を計算
		XMMATRIX model = scene.GetModelMatrix(o);

		// 定数バッファに書き込み
		XMStoreFloat4x4(&m_objectMapped[frameIndex][o]->model, XMMatrixTranspose(model));

		// オブジェクト定数をバインド
		cmdList->SetGraphicsRootConstantBufferView(1, m_objectCB[frameIndex][o]->GetGPUVirtualAddress());

		// オブジェクトごとにテクスチャを切り替え
		if (obj.textureIndex < m_srvSlot) {
			cmdList->SetGraphicsRootDescriptorTable(2, m_srvHeap.GetGPUHandle(obj.textureIndex));
		}

		// メッシュをバインドして描画
		m_meshes[obj.meshIndex].Bind(cmdList);
		m_meshes[obj.meshIndex].Draw(cmdList);
	}

	// バリア
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	cmdList->ResourceBarrier(1, &barrier);
}

int Renderer::RegisterObject(ID3D12Device* device, ID3D12CommandQueue* commandQueue, SceneObject& obj) {
	// メッシュ登録
	int meshIdx = LoadMesh(device, obj.modelPath);
	if (meshIdx<0) {
		return -1;
	}
	obj.meshIndex = meshIdx;

	// テクスチャ登録
	int texIdx = LoadTexture(device, commandQueue, obj.texturePath);
	if (texIdx < 0) {
		return -1;
	}
	obj.textureIndex=texIdx;

	return 0;
}