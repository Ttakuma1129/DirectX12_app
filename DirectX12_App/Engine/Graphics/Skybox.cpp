#include "Skybox.h"
#include "CommandContext.h"

bool Skybox::Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* allocator, const std::string faceFiles[6]) {
	HRESULT hr;

	// SRVレンジ設定
	D3D12_DESCRIPTOR_RANGE srvRange = {};
	srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRange.NumDescriptors = 1;
	srvRange.BaseShaderRegister = 0;
	srvRange.RegisterSpace = 0;
	srvRange.OffsetInDescriptorsFromTableStart = 0;

	D3D12_ROOT_PARAMETER rootParam[2] = {};

	// シーン定数(View・Proj行列)設定
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[0].Descriptor.ShaderRegister = 0;
	rootParam[0].Descriptor.RegisterSpace = 0;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// キューブマップSRV設定
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[1].DescriptorTable.pDescriptorRanges = &srvRange;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// サンプラー設定
	D3D12_STATIC_SAMPLER_DESC smpDesc = {};
	smpDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	smpDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	smpDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	smpDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	smpDesc.ShaderRegister = 0;
	smpDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	smpDesc.MaxLOD = D3D12_FLOAT32_MAX;

	// ルートシグネチャ作成
	D3D12_ROOT_SIGNATURE_DESC resDesc = {};
	resDesc.NumParameters = 2;
	resDesc.pParameters = rootParam;
	resDesc.NumStaticSamplers = 1;
	resDesc.pStaticSamplers = &smpDesc;
	resDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	hr = D3D12SerializeRootSignature(&resDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		return false;
	}

	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	if (FAILED(hr)) {
		return false;
	}

	// シェーダーコンパイル
	Microsoft::WRL::ComPtr<ID3D10Blob> vsBlob, psBlob, errorBlob;

	hr = D3DCompileFromFile(
		L"Shaders/SkyboxVS.hlsl",
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
		L"Shaders/SkyboxPS.hlsl",
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

	//パイプラインステート
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } };

	// ラスタライザ設定
	D3D12_RASTERIZER_DESC  rastDesc = {};
	rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rastDesc.CullMode = D3D12_CULL_MODE_NONE;
	rastDesc.FrontCounterClockwise = FALSE;
	rastDesc.DepthClipEnable = TRUE;

	// ブレンド設定
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// パイプラインステートオブジェクト設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
	psoDesc.InputLayout = { inputLayout, 1 };
	psoDesc.RasterizerState = rastDesc;
	psoDesc.BlendState = blendDesc;

	// デプスステンシル設定
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	// 深度バッファのフォーマットを指定
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	// トポロジータイプ
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// レンダーターゲット設定(スワップチェーンと一致させる)
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	// マルチサンプル設定
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = UINT_MAX;

	// パイプラインステートオブジェクトの作成
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

	if (FAILED(hr)) {
		return false;
	}

	// スカイボックスのキューブメッシュ
	float skyVertices[] = {
		// +X
		 1, 1,-1,   1,-1,-1,   1,-1, 1,   1, 1, 1,
		// -X
		-1, 1, 1,  -1,-1, 1,  -1,-1,-1,  -1, 1,-1,
		// +Y
		-1, 1, 1,  -1, 1,-1,   1, 1,-1,   1, 1, 1,
		// -Y
		-1,-1,-1,  -1,-1, 1,   1,-1, 1,   1,-1,-1,
		// +Z
		-1, 1, 1,   1, 1, 1,   1,-1, 1,  -1,-1, 1,
		// -Z
		 1, 1,-1,  -1, 1,-1,  -1,-1,-1,   1,-1,-1,
	};

	uint16_t skyIndices[] = {
		0,1,2,   0,2,3,
		4,5,6,   4,6,7,
		8,9,10,  8,10,11,
		12,13,14, 12,14,15,
		16,17,18, 16,18,19,
		20,21,22, 20,22,23,
	};

	// キューブメッシュ作成
	if (!m_cubeMesh.Create(device, skyVertices, sizeof(skyVertices), sizeof(float) * 3, skyIndices, 36)) {
		return false;
	}

	// SRVヒープ、キューブマップ読み込み
	if (!m_srvHeap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true)) {
		return false;
	}

	CommandContext uploadContext;
	uploadContext.Initialize(device, allocator);
	uploadContext.Begin(allocator);

	if (!m_cubeMap.Create(device, uploadContext.GetCommandList(), faceFiles, m_srvHeap.GetCPUHandle(0))) {
		return false;
	}

	uploadContext.End();
	uploadContext.Execute(commandQueue);
}