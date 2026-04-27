#pragma once
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <memory>

#include "Chunk.h"
#include "BlockType.h"

namespace WorldCoord {
	// 切り下げ除算 : 何番目のチャンクに存在しているかを計算
	inline int FloorDiv(int a, int b) {
		int q = a / b;

		// 割り切れない & 符号が異なる → 1つ下に寄せる
		if ((a % b != 0) && ((a < 0) != (b < 0))) {
			q--;
		}
		return q;
	}

	// 剰余 : チャンク内での位置を計算
	inline int Mod(int a, int b) {
		int r = a % b;
		if (r < 0) {
			r += b;
		}
		return r;
	}
}

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

// レイキャストの計算結果
struct RaycastResult {
	bool hit;	// 当たったかどうか
	int blockX; // 当たったブロックのワールド座標
	int blockY;
	int blockZ;
	int face;	// どの面に当たったか
};

class World {
public:
	// ワールド座標でのブロック取得
	BlockType GetBlock(int worldX, int worldY, int worldZ) const;

	// 面を表示するかどうか
	bool IsSolid(int worldX, int worldY, int worldZ) const;

	// Worldからチャンクを生成・登録するための関数
	Chunk* GenerateChunk(int chunkX, int chunkZ);

	// 全チャンクの取得
	const std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash>& GetChunks() const {
		return m_chunks;
	}

private:
	std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_chunks;
};