#pragma once
#include <cstdint>

struct BlockTexCoord {
	uint8_t top;
	uint8_t bottom;
	uint8_t side;
};

// ブロックごとのテクスチャ定義 (テクスチャアトラスのセル番号)
static constexpr BlockTexCoord kBlockTex[] = {
	{0, 0, 0},	// Air
	{0, 0, 0},	// Dirt
	{2, 0, 1},	// Grass
	{3, 3, 3},	// Stone
};