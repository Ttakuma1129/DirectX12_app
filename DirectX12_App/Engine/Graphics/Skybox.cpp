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
}