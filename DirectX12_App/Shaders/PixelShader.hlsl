cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
    float4 cameraPos; 
    float4 specularParams; // x:強度 y:鋭さ
};

Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct PSInput{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD1;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
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
    
    // テクスチャ色を取得
    float4 texColor = tex.Sample(smp, input.uv);
    
    // 出力される最終色
    float3 diffuse = lightColor.rgb * NdotL;
    float3 ambient = ambientColor.rgb;
    float3 finalColor = texColor.rgb * (ambient + diffuse) + lightColor.rgb * specular;
    
    return float4(finalColor, texColor.a);
}