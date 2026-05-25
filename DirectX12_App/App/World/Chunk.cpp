#include "Chunk.h"
#include "BlockTexture.h"
#include "World.h"
#include "../../Engine/ThirdParty/stb_perlin.h"

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
				else if (y <= 4){
					type = BlockType::Stone;
				}
				
				if (y >= 8) {
					type = BlockType::Air;
				}

				SetBlock(x, y, z, type);
			}
		}
	}
}

void Chunk::GenerateNoise(int chunkX, int chunkZ) {
	// ベースとなるワールド座標
	int baseWorldX = chunkX * CHUNK_SIZE;
	int baseWorldZ = chunkZ * CHUNK_SIZE;

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			int worldX = baseWorldX + x;
			int worldZ = baseWorldZ + z;

			int surfaceY = GetHeightAt(worldX, worldZ);

			for (int y = 0;y < HEIGHT; ++y) {
				BlockType type;
				if (y > surfaceY) {
					type = BlockType::Air;
				}
				else if (y == surfaceY) {
					type = BlockType::Grass;
				}
				else if (y >= surfaceY - 3) {
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

void Chunk::BuildMesh(const World& world, int chunkX, int chunkZ, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const {
	vertices.clear();
	indices.clear();

	// ベースとなるワールド座標
	int baseWorldX = chunkX * CHUNK_SIZE;
	int baseWorldZ = chunkZ * CHUNK_SIZE;

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int z = 0; z < CHUNK_SIZE; ++z) {
			for (int y = 0; y < HEIGHT; ++y) {
				if (!IsSolid(x, y, z)) {
					continue;
				}

				// ブロックタイプを取得
				BlockType type = GetBlock(x, y, z);

				// ワールド座標を計算
				int wx = baseWorldX + x;
				int wz = baseWorldZ + z;

				// ワールド座標で6方向チェック、ローカル座標で頂点生成
				if (!world.IsSolid(wx + 1, y, wz)) {
					AddFace(0, x, y, z, type, vertices, indices);
				}
				if (!world.IsSolid(wx - 1, y, wz)) {
					AddFace(1, x, y, z, type, vertices, indices);
				}
				if (!world.IsSolid(wx, y + 1, wz)) {
					AddFace(2, x, y, z, type, vertices, indices);
				}
				if (!world.IsSolid(wx, y - 1, wz)) {
					AddFace(3, x, y, z, type, vertices, indices);
				}
				if (!world.IsSolid(wx, y, wz + 1)) {
					AddFace(4, x, y, z, type, vertices, indices);
				}
				if (!world.IsSolid(wx, y, wz - 1)) {
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

void Chunk::AddFace(int faceDir, int x, int y, int z, int wx, int wz, BlockType type, const World& world, std::vector<ModelVertex>& vertices, std::vector<uint16_t>& indices) const {
	// このブロック・面が使うアトラスのセル番号取得
	uint8_t cell = GetTexIndex(type, faceDir);

	// セル番号からUV範囲を計算
	float uMin, vMin, uMax, vMax;
	GetAtlasUV(cell, uMin, vMin, uMax, vMax);
	
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
	const float faceUVs[4][2] = {
		{uMin, vMax},	// 頂点0
		{uMax, vMax},	// 頂点1
		{uMax, vMin},	// 頂点2
		{uMin, vMin},	// 頂点3
	};

	// 頂点インデックス
	uint16_t baseIndex = static_cast<uint16_t>(vertices.size());

	int ao[4];

	// 4頂点追加
	for (int i = 0;i < 4; ++i) {
		int cx = (int)faceVertices[faceDir][i][0];
		int cy = (int)faceVertices[faceDir][i][1];
		int cz = (int)faceVertices[faceDir][i][2];

		int nx = (int)faceNormals[faceDir][0];
		int ny = (int)faceNormals[faceDir][1];
		int nz = (int)faceNormals[faceDir][2];

		// 面の外側の層のワールド座標
		int bx = wx + nx;
		int by = y + ny;
		int bz = wz + nz;

		// 接線2軸のオフセット
		int s1[3] = { 0, 0, 0 };
		int s2[3] = { 0, 0, 0 };
		if (nx != 0) { // X固定 → 接線YZ
			s1[1] = (cy == 1) ? 1 : -1;
			s2[2] = (cz == 1) ? 1 : -1;
		}
		else if (ny != 0){ // Y固定 → 接線XZ
			s1[0] = (cx == 1) ? 1 : -1;
			s2[2] = (cz == 1) ? 1 : -1;
		}
		else { // Z固定 → 接線XY
			s1[0] = (cx == 1) ? 1 : -1;
			s2[2] = (cy == 1) ? 1 : -1;
		}

		int side1 = world.IsSolid(bx + s1[0], by + s1[1], bz + s1[2]) ? 1 : 0;
		int side2 = world.IsSolid(bx + s2[0], by + s2[1], bz + s2[2]) ? 1 : 0;
		int corner = world.IsSolid(bx + s1[0] + s2[0], by + s1[1] + s2[1], bz + s1[1] + s2[2]) ? 1 : 0;

		ModelVertex v = {};
		v.position[0] = faceVertices[faceDir][i][0] + x;
		v.position[1] = faceVertices[faceDir][i][1] + y;
		v.position[2] = faceVertices[faceDir][i][2] + z;
		v.normal[0] = (float)nx;
		v.normal[1] = (float)ny;
		v.normal[2] = (float)nz;
		v.uv[0] = faceUVs[i][0];
		v.uv[1] = faceUVs[i][1];
		v.ambientOcclusion = (float)ao[i];
		vertices.push_back(v);
	}

	uint16_t b = baseIndex;
	if (ao[0] + ao[2] <= ao[1] + ao[3]) {
		indices.push_back(b + 0);
		indices.push_back(b + 2);
		indices.push_back(b + 1);
		indices.push_back(b + 0);
		indices.push_back(b + 3);
		indices.push_back(b + 2);
	}
	else {
		indices.push_back(b + 1);
		indices.push_back(b + 3);
		indices.push_back(b + 2);
		indices.push_back(b + 1);
		indices.push_back(b + 0);
		indices.push_back(b + 3);
	}
}

int Chunk::GetHeightAt(int worldX, int worldZ) {
	const float scale = 0.05f;	// 地形の変化の度合(大きいほど急激に変化)
	const int minHeight = 4;
	const int maxHeight = 12;

	float nx = worldX * scale;
	float nz = worldZ * scale;
	float noise = stb_perlin_noise3(nx, 0.0f, nz, 0, 0, 0);
	float normalizedNoise = (noise + 1.0f) * 0.5f;	// 0～1の間にノイズを正規化
	int height = minHeight + static_cast<int>(normalizedNoise * (maxHeight - minHeight));

	return height;
}