#pragma once
#include <vector>
#include <cstdint>

#include "BlockType.h"
#include "../../Engine/Resources/ModelLoader.h"

class Chunk {
public:
	void GenerateFlat();

	void SetBlock(int x, int y, int z, BlockType type);

	void BuildMesh(std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const;

	// ブロック取得・設定
	BlockType GetBlock(int x, int y, int z) const;

private:
	static constexpr int CHUNK_SIZE = 16; // 1チャンクの大きさ
	static constexpr int HEIGHT = 16;

	BlockType m_blocks[CHUNK_SIZE][HEIGHT][CHUNK_SIZE] = {};
	
	bool IsSolid(int x, int y, int z) const;
	void AddFace(int faceDir, int x, int y, int z, BlockType type, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const;
};