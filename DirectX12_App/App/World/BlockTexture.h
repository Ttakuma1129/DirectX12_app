#pragma once
#include <cstdint>

#include "BlockType.h"

struct BlockTexCoord {
	uint8_t top;
	uint8_t bottom;
	uint8_t side;
};

// ブロックごとのテクスチャ定義 (テクスチャアトラスのセル番号)
static constexpr BlockTexCoord kBlockTex[] = {
// {上, 下, 横}
	{0, 0, 0},	// Air
	{0, 0, 0},	// Dirt
	{2, 0, 1},	// Grass
	{3, 3, 3},	// Stone
};

// ブロックタイプと面方向からテクスチャアトラスのセル番号を返す
static uint8_t GetTexIndex(BlockType type, int faceDir) {
	const BlockTexCoord& t = kBlockTex[static_cast<size_t>(type)];
	switch (faceDir)
	{
	case 2:
		return t.top; // 上

	case3:
		return t.bottom; // 下

	default:
		return t.side; // 横

		break;
	}
}