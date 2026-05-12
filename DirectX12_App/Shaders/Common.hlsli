#ifndef COMMON_HLSLI
#define COMMON_HLSLI

// シーン全体の定数
cbuffer SceneConstant : register(b0){
    float4x4 view;
    float4x4 proj;
    float4 lightDir;
    float4 lightColor;
    float4 ambientColor;
    float4 cameraPos;
    float4 specularParams; // x:強度 y:鋭さ
    float4x4 lightViewProj;
    float4 shadowParams; // x:バイアス y:シャドウマップサイズ
};

// オブジェクトごとの定数
cbuffer ObjectConstant : register(b1){
    float4x4 model;
};

#endif