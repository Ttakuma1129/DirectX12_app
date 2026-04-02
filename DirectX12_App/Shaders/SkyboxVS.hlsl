cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
};

struct VSInput{
    float3 position : POSITION;
};

struct VSOutput{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD;
};