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

bool World::IsSolid(int worldX, int worldY, int worldZ)const {
	return GetBlock(worldX, worldY, worldZ) != BlockType::Air;
}

Chunk* World::GenerateChunk(int chunkX, int chunkZ) {
	auto chunk = std::make_unique<Chunk>();
	chunk->GenerateNoise(chunkX, chunkZ);
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

	// 1マス進むのに必要なtの増加量
	float tDeltaX = abs(1.0f / dir.x);
	float tDeltaY = abs(1.0f / dir.y);
	float tDeltaZ = abs(1.0f / dir.z);

	// 一番近いマスの境界までにどれだけtを増やせばいいか
	float nextX = (stepX > 0) ? (x + 1) : x;
	float tNextBoundaryX = (nextX - origin.x) / dir.x;
	float nextY = (stepY > 0) ? (y + 1) : y;
	float tNextBoundaryY = (nextY - origin.y) / dir.y;
	float nextZ = (stepZ > 0) ? (z + 1) : z;
	float tNextBoundaryZ = (nextZ - origin.z) / dir.z;

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
			result;
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