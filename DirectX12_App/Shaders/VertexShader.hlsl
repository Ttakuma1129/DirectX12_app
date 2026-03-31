// 定数バッファ
cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
    float4 cameraPos;
    float4 specularParams; // x:強度 y:鋭さ
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
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD1;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VSOutput main(VSInput input){
    VSOutput output;
    // 行列計算
    float4 worldPos = mul(float4(input.pos, 1.0), model); 
    output.worldPos = worldPos.xyz;
    
    float4 viewPos = mul(worldPos, view); 
    output.pos = mul(viewPos, proj); // Projection行列での変換
    
    // 法線をワールド空間座標に変換
    output.normal = mul(input.normal, (float3x3) model);
    
    output.uv = input.uv;
    return output;
}