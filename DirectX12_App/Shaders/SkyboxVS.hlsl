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

VSOutput main(VSInput input){
    VSOutput output;
    
    // 視線方向をテクスチャ座標として扱う
    output.texCoord = input.position;
    
    // View行列から平行移動を除外
    float4x4 viewNoTranslation = view;
    viewNoTranslation[3] = float4(0, 0, 0, 1);
    
    float4 pos = mul(float4(input.position, 1.0), viewNoTranslation);
    pos = mul(pos, proj);
    
    // 常に最奥に表示されるように深度を最大値に固定
    output.position = pos.xyww;
    
    return output;
}