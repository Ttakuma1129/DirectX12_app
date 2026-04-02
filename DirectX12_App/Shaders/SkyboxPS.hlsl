TextureCube skyTexture : register(t0);
SamplerState smp : register(s0);

struct PSInput{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD;
};