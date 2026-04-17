#include "Chunk.h"

void Chunk::GenerateFlat() {
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			for (int y = 0; y < CHUNK_SIZE; ++y) {
				BlockType type;
				if (y == 7) {
					type = BlockType::Grass;
				}
				else if (y >= 5) {
					type = BlockType::Dirt;
				}
				else {
					type = BlockType::Stone;
				}
				SetBlock(x, y, z, type);
			}
		}
	}
}

void Chunk::SetBlock(int x, int y, int z, BlockType type) {
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= HEIGHT || z < 0 || z >= CHUNK_SIZE) {
		return;
	}
	m_blocks[x][y][z] = type;
}

BlockType Chunk::GetBlock(int x, int y, int z) const {
	if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= HEIGHT || z < 0 || z >= CHUNK_SIZE) {
		return BlockType::Air;
	}
	return m_blocks[x][y][z];
}

bool Chunk::IsSolid(int x, int y, int z) const {
	return GetBlock(x, y, z) != BlockType::Air;
}