#include "Chunk.h"

void Chunk::GenerateFlat() {
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			for (int y = 0; y < HEIGHT; ++y) {
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

void Chunk::BuildMesh(std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const {
	vertices.clear();
	indices.clear();
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			for (int y = 0; y < CHUNK_SIZE; ++y) {
				if (!IsSolid(x, y, z)) {
					continue;
				}

				// ブロックタイプを取得
				BlockType type = GetBlock(x, y, z);

				// 6方向チェック
				if (!IsSolid(x + 1, y, z)) {
					AddFace(0, x, y, z, type, vertices, indices);
				}
				if (!IsSolid(x - 1, y, z)) {
					AddFace(1, x, y, z, type, vertices, indices);
				}
				if (!IsSolid(x, y + 1, z)) {
					AddFace(2, x, y, z, type, vertices, indices);
				}
				if (!IsSolid(x, y - 1, z)) {
					AddFace(3, x, y, z, type, vertices, indices);
				}
				if (!IsSolid(x, y, z + 1)) {
					AddFace(4, x, y, z, type, vertices, indices);
				}
				if (!IsSolid(x, y, z - 1)) {
					AddFace(5, x, y, z, type, vertices, indices);
				}
			}
		}
	}
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

void Chunk::AddFace(int faceDir, int x, int y, int z, BlockType type, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const {
	// 方向事の頂点位置オフセット(ブロックの原点基準)
	static const float faceVertices[6][4][3] = {
		{{1,0,0},{1,0,1},{1,1,1},{1,1,0}}, // 右
		{{0,0,1},{0,0,0},{0,1,0},{0,1,1}}, // 左
		{{0,1,0},{1,1,0},{1,1,1},{0,1,1}}, // 上
		{{0,0,1},{1,0,1},{1,0,0},{0,0,0}}, // 下
		{{1,0,1},{0,0,1},{0,1,1},{1,1,1}}, // 前
		{{0,0,0},{1,0,0},{1,1,0},{0,1,0}}, // 後
	};

	// 面ごとの法線
	static const float faceNormals[6][3] = {
		{1,0,0}, {-1,0,0},
		{0,1,0}, {0,-1,0},
		{0,0,1}, {0,0,-1},
	};

	// UV座標
	static const float faceUVs[4][2] = {
		{0,1},{1,1},{1,0},{0,0},
	};

	// 頂点インデックス
	uint16_t baseIndex = static_cast<uint16_t>(vertices.size());

	// 4頂点追加
	for (int i = 0;i < 4; ++i) {
		ModelVertex v = {};
		v.position[0] = faceVertices[faceDir][i][0] + x;
		v.position[1] = faceVertices[faceDir][i][1] + y;
		v.position[2] = faceVertices[faceDir][i][2] + z;
		v.normal[0] = faceNormals[faceDir][0];
		v.normal[1] = faceNormals[faceDir][1];
		v.normal[2] = faceNormals[faceDir][2];
		v.uv[0] = faceUVs[i][0];
		v.uv[1] = faceUVs[i][1];
		vertices.push_back(v);
	}

	indices.push_back(baseIndex + 0);
	indices.push_back(baseIndex + 2);
	indices.push_back(baseIndex + 1);
	indices.push_back(baseIndex + 0);
	indices.push_back(baseIndex + 3);
	indices.push_back(baseIndex + 2);
}