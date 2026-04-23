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