#include "PipelineState.h"

bool PipelineState::Initialize(
	ID3D12Device* device, ID3D12RootSignature* rootSignature,
	const void* vsData, size_t vsSize, const void* psData, size_t psSize) {

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
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			12,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		}
	};

	// ラスタライザ設定
	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	
	// ブレンド設定
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//パイプラインステートオブジェクト設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	// シェーダー
	psoDesc.VS = { vsData,vsSize };
	psoDesc.PS = { psData,psSize };

	// ルートシグネチャ
	psoDesc.pRootSignature = rootSignature;

	// 入力レイアウト
	psoDesc.InputLayout = {
		inputLayout,
		_countof(inputLayout)
	};

	// ラスタライザ ブレンド
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = blendDesc;

	// DepthStencil
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;

	// トポロジータイプ
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	// レンダーターゲット設定(スワップチェーンと一致させる)
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	// マルチサンプル設定
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = UINT_MAX;

	// パイプラインステートオブジェクトの作成
	HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));

	if (FAILED(hr)) {
		return false;
	}
	return true;
}