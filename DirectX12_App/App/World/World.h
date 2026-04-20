#pragma once
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <memory>

#include "Chunk.h"
#include "BlockType.h"

// チャンクごとの位置を表す座標
struct ChunkCoord{
	int x, z;
	bool operator==(const ChunkCoord& o)const {
		return x == o.x && z == o.z;
	}
};

// unorderd_mapのキーに使うためのハッシュ関数
struct ChunkCoordHash {
	size_t operator() (const ChunkCoord& c) const {
		return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 16);
	}
};

class World {
public:
	// ワールド座標でのブロック取得
	BlockType GetBlock(int worldX, int worldY, int worldZ) const;

	// 面を表示するかどうか
	bool IsSolid(int worldX, int worldY, int worldZ) const;

private:
	std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_chunks;
};