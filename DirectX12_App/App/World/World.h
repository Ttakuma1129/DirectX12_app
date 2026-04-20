#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include "Chunk.h"
#include "BlockType.h"

// チャンクごとの座標
struct ChunkCoord{
	int x, z;
	bool operator==(const ChunkCoord& o)const {
		return x == o.x && z == o.z;
	}
};

// unorderd_mapのキーに使うためのハッシュ
struct ChunkCoordHash {
	size_t operator() (const ChunkCoord& c) const {
		return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 16);
	}
};

class World {
public:
	// チャンクを生成して登録
	Chunk* GenerateChunk(int chunkX, int chunkZ);

	// ワールド座標でのブロック取得
	BlockType GetBlock(int worldX, int worldY, int worldZ) const;

	// 面を表示するかどうか
	bool IsSolid(int worldX, int worldY, int worldZ) const {
		return GetBlock(worldX, worldY, worldZ) != BlockType::Air;
	}

	// 全チャンクに対してイテレート
	const auto& GetChunk() const {
		return m_chunks;
	}

private:
	std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_chunks;
};