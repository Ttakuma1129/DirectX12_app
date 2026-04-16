#include "Chunk.h"

BlockType Chunk::GetBlock(int x, int y, int z) const {
	if (x < 0 || x >= SIZE || y < 0 || y >= HEIGHT || z < 0 || z >= SIZE) {
		return BlockType::Air;
	}
	return m_blocks[x][y][z];
}