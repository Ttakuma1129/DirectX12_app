#include "World.h"

BlockType World::GetBlock(int worldX, int worldY, int worldZ)const {


	return BlockType::Air;
}

bool World::IsSolid(int worldX, int worldY, int worldZ)const {
	return GetBlock(worldX, worldY, worldZ) != BlockType::Air;
}