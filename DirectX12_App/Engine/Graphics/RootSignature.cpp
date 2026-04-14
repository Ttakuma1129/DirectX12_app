#include "RootSignature.h"

bool RootSignature::Initialize(ID3D12Device* device) {

	// SRVレンジを設定
	D3D12_DESCRIPTOR_RANGE srvRange = {};
	srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	srvRange.NumDescriptors = 1;
	srvRange.BaseShaderRegister = 0; //t0
	srvRange.RegisterSpace = 0;
	srvRange.OffsetInDescriptorsFromTableStart = 0;

	// シャドウマップ用SRVレンジを設定
	D3D12_DESCRIPTOR_RANGE shadowSrvRange = {};
	shadowSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	shadowSrvRange.NumDescriptors = 1;
	shadowSrvRange.BaseShaderRegister = 1; // t1
	shadowSrvRange.RegisterSpace = 0;
	shadowSrvRange.OffsetInDescriptorsFromTableStart = 0;

	// ルートパラメータの数
	D3D12_ROOT_PARAMETER rootParam[4] = {};

	// シーン行列(VP行列)、ライト情報をルートパラメータとして追加
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[0].Descriptor.ShaderRegister = 0;
	rootParam[0].Descriptor.RegisterSpace = 0;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// オブジェクト定数(M行列)をルートパラメータとして追加
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[1].Descriptor.ShaderRegister = 1;
	rootParam[1].Descriptor.RegisterSpace = 0;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// SRVのDescriptorTableをルートパラメータとして追加
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[2].DescriptorTable.pDescriptorRanges = &srvRange;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// シャドウマップのSRVをルートパラメータとして追加
	rootParam[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[2].DescriptorTable.pDescriptorRanges = &shadowSrvRange;
	rootParam[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// StaticSamplerの設定
	D3D12_STATIC_SAMPLER_DESC smpDesc = {};
	smpDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	smpDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	smpDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	smpDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	smpDesc.ShaderRegister = 0;
	smpDesc.RegisterSpace = 0;
	smpDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	smpDesc.MaxLOD = D3D12_FLOAT32_MAX;

	// シャドウサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC shadowSmpDesc = {};
	shadowSmpDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowSmpDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	shadowSmpDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	shadowSmpDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	shadowSmpDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	shadowSmpDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	shadowSmpDesc.ShaderRegister = 1;
	shadowSmpDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	shadowSmpDesc.MaxLOD = D3D12_FLOAT32_MAX;

	// サンプラーをまとめる
	D3D12_STATIC_SAMPLER_DESC samplers[] = { smpDesc, shadowSmpDesc };

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = 3;
	desc.pParameters = rootParam;
	desc.NumStaticSamplers = 1;
	desc.pStaticSamplers = &smpDesc;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// シリアライズ
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob
	);
	if (FAILED(hr)) {
		return false;
	}
	
	// GPUオブジェクト作成
	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}