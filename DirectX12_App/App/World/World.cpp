#include <cfloat>
#include <fstream>

#include "World.h"

BlockType World::GetBlock(int worldX, int worldY, int worldZ)const {
	// Yが範囲外ならAirにする
	if (worldY < 0 || worldY >= Chunk::HEIGHT) {
		return BlockType::Air;
	}

	// ワールド座標をチャンク座標に変換
	int chunkX = WorldCoord::FloorDiv(worldX, Chunk::CHUNK_SIZE);
	int chunkZ = WorldCoord::FloorDiv(worldZ, Chunk::CHUNK_SIZE);

	// そのブロックが存在しているチャンクを探す
	auto it = m_chunks.find({ chunkX, chunkZ });
	if (it == m_chunks.end()) {
		return BlockType::Air;	// 見つからなかった場合airを返す
	}

	// ワールド座標をローカル座標に変換
	int localX = WorldCoord::Mod(worldX, Chunk::CHUNK_SIZE);
	int localZ = WorldCoord::Mod(worldZ, Chunk::CHUNK_SIZE);

	return it->second->GetBlock(localX, worldY, localZ);
}

void World::SetBlockAt(int worldX, int worldY, int worldZ, BlockType type) {
	// どのチャンクに存在しているかを取得
	int chunkX = WorldCoord::FloorDiv(worldX, Chunk::CHUNK_SIZE);
	int chunkZ = WorldCoord::FloorDiv(worldZ, Chunk::CHUNK_SIZE);
	// チャンク内のローカル座標での位置を計算
	int localX = WorldCoord::Mod(worldX, Chunk::CHUNK_SIZE);
	int localZ = WorldCoord::Mod(worldZ, Chunk::CHUNK_SIZE);

	if (worldY < 0 || worldY >= Chunk::HEIGHT) {
		return;
	}

	// ロード中のチャンクに適応
	auto it = m_chunks.find({ chunkX, chunkZ });
	if (it != m_chunks.end()) {
		it->second->SetBlock(localX, worldY, localZ, type);
	}

	// 差分を記録
	auto& edits = m_edits[{chunkX, chunkZ}];
	for (auto& e : edits) {
		if (e.x == localX && e.y == worldY && e.z == localZ) {
			e.type = type;
			return;
		}
	}

	edits.push_back({ localX, worldY, localZ, type });
}

bool World::IsSolid(int worldX, int worldY, int worldZ)const {
	return GetBlock(worldX, worldY, worldZ) != BlockType::Air;
}

Chunk* World::GenerateChunk(int chunkX, int chunkZ) {
	auto chunk = std::make_unique<Chunk>();
	chunk->GenerateNoise(chunkX, chunkZ, m_seed);

	// 過去の編集を再適用
	auto it = m_edits.find({ chunkX,chunkZ });
	if (it != m_edits.end()) {
		for (const auto& e : it->second) {
			chunk->SetBlock(e.x, e.y, e.z, e.type);
		}
	}

	Chunk* ptr = chunk.get();
	m_chunks[{chunkX, chunkZ}] = std::move(chunk);
	return ptr;
}

RaycastResult World::Raycast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& dir, float maxDist) const {
	using namespace DirectX;

	RaycastResult result = {};
	result.hit = false;

	// レイキャスト現在地を開始位置で初期化
	int x = (int)floor(origin.x);
	int y = (int)floor(origin.y);
	int z = (int)floor(origin.z);

	// 進行方向
	int stepX = (dir.x > 0) ? 1 : -1;
	int stepY = (dir.y > 0) ? 1 : -1;
	int stepZ = (dir.z > 0) ? 1 : -1;

	// 軸に正対しているかの判定用のイプシロン
	const float EPS = 1e-8f;

	// 1マス進むのに必要なtの増加量
	float tDeltaX = (fabsf(dir.x) < EPS) ? FLT_MAX : fabsf(1.0f / dir.x);
	float tDeltaY = (fabsf(dir.y) < EPS) ? FLT_MAX : fabsf(1.0f / dir.y);
	float tDeltaZ = (fabsf(dir.z) < EPS) ? FLT_MAX : fabsf(1.0f / dir.z);

	// 一番近いマスの境界までにどれだけtを増やせばいいか
	float nextX = (stepX > 0) ? (x + 1) : x;
	float tNextBoundaryX = (fabsf(dir.x) < EPS) ? FLT_MAX : (nextX - origin.x) / dir.x;
	float nextY = (stepY > 0) ? (y + 1) : y;
	float tNextBoundaryY = (fabsf(dir.y) < EPS) ? FLT_MAX : (nextY - origin.y) / dir.y;
	float nextZ = (stepZ > 0) ? (z + 1) : z;
	float tNextBoundaryZ = (fabsf(dir.z) < EPS) ? FLT_MAX : (nextZ - origin.z) / dir.z;

	int hitFace = -1; // レイキャストが当たった面の方向
	float t = 0.0f;
	const int maxIterations = 100; // 無限ループ防止

	for (int i = 0; i < maxIterations; ++i) {
		// 現在のマスにブロックがあるかをチェック
		if (IsSolid(x, y, z)) {
			result.hit = true;
			result.blockX = x;
			result.blockY = y;
			result.blockZ = z;
			result.face = hitFace;
			return result;
		}

		// 最小のtNextBoundaryを選びその方向に1マス進む
		if (tNextBoundaryX < tNextBoundaryY) {
			if (tNextBoundaryX < tNextBoundaryZ) {
				// xが最小
				x += stepX;	// マスを1つ進める
				t = tNextBoundaryX; // tに境界までの距離を保存
				tNextBoundaryX += tDeltaX; // 次のマスの境界までの距離を更新
				hitFace = (stepX > 0) ? 1 : 0; // どの面にレイが当たるか
			}
			else {
				// Zが最小
				z += stepZ;
				t = tNextBoundaryZ;
				tNextBoundaryZ += tDeltaZ;
				hitFace = (stepZ > 0) ? 5 : 4;
			}
		}
		else {
			if (tNextBoundaryY < tNextBoundaryZ) {
				// yが最小
				y += stepY;
				t = tNextBoundaryY;
				tNextBoundaryY += tDeltaY;
				hitFace = (stepY > 0) ? 3 : 2;
			}
			else {
				// Zが最小
				z += stepZ;
				t = tNextBoundaryZ;
				tNextBoundaryZ += tDeltaZ;
				hitFace = (stepZ > 0) ? 5 : 4;
			}
		}
		
		// レイの長さが最大値を超えたらループを抜ける
		if (t > maxDist) {
			break;
		}
	}
	return result;
}

bool World::SaveToFile(const std::string& path, const DirectX::XMFLOAT3& playerPos)const {
	std::ofstream file(path, std::ios::binary);
	if (!file) {
		return false;
	}

	// バージョン、シード値、プレイヤーの位置を書き込む
	const char magic[4] = { 'V','X','L','W' };
	uint32_t version = 2;
	file.write(magic, 4);
	file.write(reinterpret_cast<const char*>(&version), sizeof(version));
	file.write(reinterpret_cast<const char*>(&m_seed), sizeof(m_seed));
	file.write(reinterpret_cast<const char*>(&playerPos.x), sizeof(float));
	file.write(reinterpret_cast<const char*>(&playerPos.y), sizeof(float));
	file.write(reinterpret_cast<const char*>(&playerPos.z), sizeof(float));

	// チャンクカウントを書き込む
	uint32_t chunkCount = static_cast<uint32_t>(m_edits.size());
	file.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

	// 編集された位置とブロック情報を書き込む
	for (const auto& [coord, edits] : m_edits) {
		int32_t cx = coord.x, cz = coord.z;
		uint32_t editCount = static_cast<uint32_t>(edits.size());
		file.write(reinterpret_cast<const char*>(&cx), sizeof(cx));
		file.write(reinterpret_cast<const char*>(&cz), sizeof(cz));
		file.write(reinterpret_cast<const char*>(&editCount), sizeof(editCount));

		for (const auto& e : edits) {
			int32_t x = e.x, y = e.y, z = e.z;
			uint32_t type = static_cast<uint32_t>(e.type);
			file.write(reinterpret_cast<const char*>(&x), sizeof(x));
			file.write(reinterpret_cast<const char*>(&y), sizeof(y));
			file.write(reinterpret_cast<const char*>(&z), sizeof(z));
			file.write(reinterpret_cast<const char*>(&type), sizeof(type));
		}
	}
	return static_cast<bool>(file);
}

bool World::LoadFromFile(const std::string& path, DirectX::XMFLOAT3& outPlayerPos) {
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		return false;
	}

	char magic[4];
	uint32_t version;
	file.read(magic, 4);
	file.read(reinterpret_cast<char*>(&version), sizeof(version));
	if (!file || magic[0] != 'V' || magic[1] != 'X' || magic[2] != 'L'|| magic[3] != 'W') {
		return false;
	}
	if (version != 2) {
		return false;
	}

	uint32_t seed = 0;
	file.read(reinterpret_cast<char*>(&seed), sizeof(seed));
	if (!file) {
		return false;
	}

	// 現在のワールドをクリア
	m_chunks.clear();
	m_edits.clear();
	m_seed = seed;

	// プレイヤーの位置を読み込む
	file.read(reinterpret_cast<char*>(&outPlayerPos.x), sizeof(float));
	file.read(reinterpret_cast<char*>(&outPlayerPos.y), sizeof(float));
	file.read(reinterpret_cast<char*>(&outPlayerPos.z), sizeof(float));

	// チャンク情報と編集情報を読み込む
	uint32_t chunkCount = 0;
	file.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));
	if (!file) {
		return false;
	}

	for (uint32_t i = 0; i < chunkCount; ++i) {
		int32_t cx, cz;
		uint32_t editCount;
		file.read(reinterpret_cast<char*>(&cx), sizeof(cx));
		file.read(reinterpret_cast<char*>(&cz), sizeof(cz));
		file.read(reinterpret_cast<char*>(&editCount), sizeof(editCount));
		if (!file) {
			return false;
		}

		std::vector<LocalBlockEdit> edits;
		edits.reserve(editCount);
		for (uint32_t j = 0; j < editCount; ++j) {
			int32_t x, y, z;
			uint32_t type;
			file.read(reinterpret_cast<char*>(&x), sizeof(x));
			file.read(reinterpret_cast<char*>(&y), sizeof(y));
			file.read(reinterpret_cast<char*>(&z), sizeof(z));
			file.read(reinterpret_cast<char*>(&type), sizeof(type));
			if (!file) {
				return false;
			}
			edits.push_back({ x,y,z,static_cast<BlockType>(type) });
		}
		m_edits[{cx, cz}] = std::move(edits);
	}
	return true;
}