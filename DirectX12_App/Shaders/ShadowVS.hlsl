#include "Common.hlsli"

struct VSInput{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput{
    float4 pos : SV_POSITION;
};

VSOutput main(VSInput input){
    VSOutput output;
    // ライトの視点からみた位置を計算
    float4 worldPos = mul(float4(input.pos, 1.0), model);
    output.pos = mul(worldPos, lightViewProj);
    return output;
}