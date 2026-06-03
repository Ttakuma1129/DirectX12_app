#pragma once
#include <string>
#include <cstdint>

// オブジェクト1つ分のデータ
struct SceneObject {
	std::string modelPath;	// モデルのファイルパス
	std::string texturePath = "app/Textures/sample.png"; // テクスチャのパス
	uint32_t meshIndex = 0; // Rendererで使うメッシュ番号
	uint32_t textureIndex = 0; // Rendererで使うテクスチャ番号
	float position[3] = { 0,0,0 }; //オブジェクトの位置
	float rotation[3] = { 0,0,0 }; // オブジェクトの回転(ラジアン)
	float scale = 1.0f;
	bool autoRotate = false;
	char name[32] = "Object"; // ImGuiで表示する名前
};