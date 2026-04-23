#pragma once
#include <vector>
#include <cstdint>

#include "BlockType.h"
#include "../../Engine/Resources/ModelLoader.h"

class World;

class Chunk {
public:
	static constexpr int CHUNK_SIZE = 16; // 1チャンクの大きさ
	static constexpr int HEIGHT = 16;

	// 平らな地形を生成
	void GenerateFlat();
	
	// パーリンノイズで地形を生成
	void GenerateNoise(int chunkX, int chunkZ);

	void SetBlock(int x, int y, int z, BlockType type);

	void BuildMesh(const World& world, int chunkX, int chunkZ, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const;

	// ブロック取得・設定
	BlockType GetBlock(int x, int y, int z) const;

private:
	BlockType m_blocks[CHUNK_SIZE][HEIGHT][CHUNK_SIZE] = {};
	
	bool IsSolid(int x, int y, int z) const;
	void AddFace(int faceDir, int x, int y, int z, BlockType type, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const;
	
	// パーリンノイズを使用して座標ごとの高さ(Y)を返す
	int GetHeightAt(int worldX, int worldZ);
};