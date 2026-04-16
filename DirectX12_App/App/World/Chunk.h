#pragma once
#include <vector>
#include <cstdint>

#include "BlockType.h"
#include "../../Engine/Resources/ModelLoader.h"

class Chunk {
public:
	void GenerateFlat();

	void SetBlock(int x, int y, int z) const;

	void BuildMesh(std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const;

	// ブロック取得・設定
	BlockType GetBlock(int x, int y, int z) const;

private:
	static constexpr int SIZE = 16;
	static constexpr int HEIGHT = 16;

	BlockType m_blocks[SIZE][HEIGHT][SIZE] = {};
	
	bool IsSolid(int x, int y, int z) const;
	void AddFace(int faceDir, int x, int y, int z, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const;
};