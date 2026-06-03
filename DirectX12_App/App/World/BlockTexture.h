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
inline uint8_t GetTexIndex(BlockType type, int faceDir) {
	const BlockTexCoord& t = kBlockTex[static_cast<size_t>(type)];
	switch (faceDir)
	{
	case 2:
		return t.top; // 上

	case 3:
		return t.bottom; // 下

	default:
		return t.side; // 横

		break;
	}
}

// アトラス全体の定数
static constexpr int ATLAS_COLS = 4; // 横方向のセルの数
static constexpr int ATLAS_ROWS = 4; // 縦方向のセルの数

// セル番号からUV範囲を計算
inline void GetAtlasUV(uint8_t cellIndex, float& uMin, float& vMin, float& uMax, float& vMax) {
	int col = cellIndex % ATLAS_COLS;
	int row = cellIndex / ATLAS_COLS;

	uMin = static_cast<float>(col) / ATLAS_COLS;
	vMin = static_cast<float>(row) / ATLAS_ROWS;
	uMax = static_cast<float>(col + 1) / ATLAS_COLS;
	vMax = static_cast<float>(row + 1) / ATLAS_ROWS;
}