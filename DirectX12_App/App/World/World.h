#pragma once
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

	}
};

