// 定数バッファ
cbuffer SceneConstant : register(b0){ 
    float4x4 mvp;
}

struct VSInput{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input){
    VSOutput output;
    // MVP行列で変換
    output.pos = mul(mvp, float4(input.pos, 1.0f));
    output.color = input.color;
    return output;
}