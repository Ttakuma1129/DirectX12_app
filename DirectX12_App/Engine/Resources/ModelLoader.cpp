#include "ModelLoader.h"
#include "../ThirdParty/tiny_obj_loader.h"
#include <unordered_map>

// 頂点の一致判定用ハッシュ
struct VertexKey{
	int posIndex;
	int uvIndex;

	bool operator == (const VertexKey& other) const {
		return posIndex == other.posIndex && uvIndex == other.uvIndex;
	}
};

struct VertexKeyHash{
	size_t operator()(const VertexKey& k) const {
		return std::hash<int>()(k.posIndex) ^ (std::hash<int>()(k.uvIndex) << 16);
	}
};