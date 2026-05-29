#pragma once
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <memory>
#include <DirectXMath.h>

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

// 編集された1ブロック分の情報(チャンク内ローカル座標)
struct LocalBlockEdit{
	int x, y, z;
	BlockType type;
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

	// ワールド座標でブロック設置
	void SetBlockAt(int worldX, int worldY, int worldZ, BlockType type);

	// 面を表示するかどうか
	bool IsSolid(int worldX, int worldY, int worldZ) const;

	// チャンクが存在しているか
	bool HasChunk(const ChunkCoord& coord) const {
		return m_chunks.find(coord) != m_chunks.end();
	}

	// 該当チャンクを消去
	void RemoveChunk(const ChunkCoord& coord) {
		m_chunks.erase(coord);
	}

	// 該当チャンクのポインタを取得
	Chunk* GetChunkPtr(const ChunkCoord& coord) {
		auto it = m_chunks.find(coord);
		return (it == m_chunks.end()) ? nullptr : it->second.get();
	}

	// Worldからチャンクを生成・登録するための関数
	Chunk* GenerateChunk(int chunkX, int chunkZ);

	RaycastResult Raycast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& dir, float maxDist) const;

	// 全チャンクの取得
	const std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash>& GetChunks() const {
		return m_chunks;
	}

	void SetSeed(uint32_t seed) {
		m_seed = seed;
	}

	uint32_t GetSeed() {
		return m_seed;
	}

	void ClearChunks() {
		m_chunks.clear();
	}

private:
	uint32_t m_seed = 0;

	std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_chunks;
	std::unordered_map<ChunkCoord, std::vector<LocalBlockEdit>, ChunkCoordHash> m_edits;
};