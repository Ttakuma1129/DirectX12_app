#pragma once
#include <d3d12.h>
#include <vector>
#include <string>
#include <cstdint>

struct ModelVertex {
	float position[3];
	float normal[3];
	float uv[2];
	float ambientOcclusion;
};

struct ModelData{
	std::vector<ModelVertex> vertices;
	std::vector<uint16_t> indices;
};

class ModelLoader {
public:
	// OBJファイルを読み込んで頂点・インデックスデータに変換する
	static bool LoadOBJ(const std::string& flilepath, ModelData& outData);
};