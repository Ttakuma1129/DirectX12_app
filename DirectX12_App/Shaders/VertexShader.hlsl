struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    // float3‚©‚çfloat4‚É•ÏŠ·
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
    return output;
}