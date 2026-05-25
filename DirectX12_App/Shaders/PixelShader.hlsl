#include "Common.hlsli"

Texture2D tex : register(t0);
Texture2D shadowMap : register(t1);
SamplerState smp : register(s0);
SamplerComparisonState shadowSampler : register(s1);

struct PSInput{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD1;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float ambientOcclusion : TEXCOORD2;
};

float4 main(PSInput input) : SV_TARGET{
    // 法線を正規化する
    float3 N = normalize(input.normal);
    
    // ライト方向を正規化　(CPU側から渡される方向の逆が光が来る方向)
    float3 L = normalize(-lightDir.xyz);
    
    // ランバート反射
    float NdotL = max(0.0, dot(N, L));
    
    // スペキュラー反射
    float3 V = normalize(cameraPos.xyz - input.worldPos); // 視線方向のベクトル
    float3 H = normalize(L + V); // ハーフベクトル
    float NdotH = max(0.0, dot(N, H));
    float specular = pow(NdotH, specularParams.y) * specularParams.x;
    
    // シャドウテスト
    float4 posInLight = mul(float4(input.worldPos, 1.0), lightViewProj);
    float3 projCoords = posInLight.xyz / posInLight.w;
    
    // [-1,1] → [0,1] に変換
    float2 shadowUV = float2(projCoords.x * 0.5 + 0.5, -projCoords.y * 0.5 + 0.5);
    
    // 比較サンプリング
    float bias = shadowParams.x;
    float shadowMapSize = shadowParams.y;
    float texelSize = 1.0 / shadowMapSize;
    
    // 3x3PCF 周囲9点をサンプリングして平均 
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x){
        for (int y = -1; y <= 1; ++y) {
            float2 offset = float2(x, y) * texelSize;
            shadow += shadowMap.SampleCmpLevelZero(shadowSampler, shadowUV + offset, projCoords.z - bias);
        }

    }
    shadow /= 9.0;
    
    // シャドウマップ範囲外は影なし
        if (projCoords.z > 1.0 || projCoords.z < 0.0){
            shadow = 1.0;
        }
    
    // テクスチャ色を取得
    float4 texColor = tex.Sample(smp, input.uv);
    
    // 出力される最終色
    float3 diffuse = lightColor.rgb * NdotL;
    float3 ambient = ambientColor.rgb;
    float3 finalColor = texColor.rgb * (ambient + diffuse * shadow) + lightColor.rgb * specular * shadow;
    
    return float4(finalColor, texColor.a);
}