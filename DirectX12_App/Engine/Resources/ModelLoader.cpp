#include "ModelLoader.h"
#include "../ThirdParty/tiny_obj_loader.h"
#include <unordered_map>

// 頂点の一致判定用ハッシュ
struct VertexKey{
	int posIndex;
	int normalIndex;
	int uvIndex;

	bool operator == (const VertexKey& other) const {
		return posIndex == other.posIndex && normalIndex == other.normalIndex && uvIndex == other.uvIndex;
	}
};

struct VertexKeyHash{
	size_t operator()(const VertexKey& k) const {
		return std::hash<int>()(k.posIndex) ^ (std::hash<int>()(k.normalIndex) << 11) ^ (std::hash<int>()(k.uvIndex) << 22);
	}
};

bool ModelLoader::LoadOBJ(const std::string& filepath, ModelData& outData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	// OBJファイルを読み込む
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());
	if (!ret) {
		OutputDebugStringA(("OBJ load failed:" + err + "\n").c_str());
		return false;
	}

	outData.vertices.clear();
	outData.indices.clear();

	// 頂点の重複を排除するためのマップ
	std::unordered_map<VertexKey, uint16_t, VertexKeyHash> vertexMap;

	// 全シェイプの全フェイスを処理
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			VertexKey key = { index.vertex_index, index.normal_index, index.texcoord_index };

			//同じ頂点が既にあればインデックスを再利用
			auto it = vertexMap.find(key);
			if (it != vertexMap.end()) {
				outData.indices.push_back(it->second);
			}
			else {
				// 新しい頂点を作成
				ModelVertex vertex = {};

				// 位置
				vertex.position[0] = attrib.vertices[3 * index.vertex_index + 0];
				vertex.position[1] = attrib.vertices[3 * index.vertex_index + 1];
				vertex.position[2] = attrib.vertices[3 * index.vertex_index + 2];

				// 法線を設定
				if (index.normal_index >= 0) {
					vertex.normal[0] = attrib.normals[3 * index.normal_index + 0];
					vertex.normal[1] = attrib.normals[3 * index.normal_index + 1];
					vertex.normal[2] = attrib.normals[3 * index.normal_index + 2];
				}
				else {
					vertex.normal[0] = 0.0f;
					vertex.normal[1] = 1.0f;
					vertex.normal[2] = 0.0f;
				}

				// UVがあれば使う、なければ0にする
				if (index.texcoord_index >= 0) {
					vertex.uv[0] = attrib.texcoords[2 * index.texcoord_index + 0];
					vertex.uv[1] = 1.0f - attrib.texcoords[2 * index.texcoord_index + 0];
				}
				else {
					vertex.uv[0] = 0.0f;
					vertex.uv[1] = 0.0f;
				}

				uint16_t newIndex = static_cast<uint16_t>(outData.vertices.size());
				outData.vertices.push_back(vertex);
				outData.indices.push_back(newIndex);
				vertexMap[key] = newIndex;
			}
		}
	}

	// 結果をデバッグ出力
	char buffer[256];
	sprintf_s(buffer, "OBJ loaded: %zu vertices, %zu indices\n", outData.vertices.size(), outData.indices.size());
	OutputDebugStringA(buffer);

	return true;
}