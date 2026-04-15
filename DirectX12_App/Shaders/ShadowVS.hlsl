cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
    float4 cameraPos;
    float4 specularPrams;
    float4x4 lightViewProj;
    float4 shadowParams;
};

cbuffer ObjectConstant : register(b1){
    float4x4 model;
};

struct VSInput{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput{
    float pos : SV_POSITION;
};

VSOutput main(VSInput input){
    VSOutput output;
    // ライトの視点からみた位置を計算
    float4 worldPos = mul(float4(input.pos, 1.0), model);
    output.pos = mul(worldPos, lightViewProj);
    return output;
}