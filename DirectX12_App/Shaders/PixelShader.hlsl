cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColot;
}

Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct PSInput{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET{
    return tex.Sample(smp,input.uv);
}