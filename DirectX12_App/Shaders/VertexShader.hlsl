// 定数バッファ
cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColot;
};

cbuffer ObjectConstant : register(b1){
    float4x4 model;
};

struct VSInput{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input){
    VSOutput output;
    // MVP行列で変換
    output.pos = mul(float4(input.pos, 1.0f), model); // Model行列での変換
    output.pos = mul(output.pos, view); // View行列での変換
    output.pos = mul(output.pos, proj); // Projection行列での変換
    output.uv = input.uv;
    return output;
}