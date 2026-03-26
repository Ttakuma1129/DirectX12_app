#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <cstdint>

class Scene {
public:
	void Initialize(float aspectRatio);
	void Update();

	DirectX::XMMATRIX GetViewMatrix() const;

	DirectX::XMMATRIX GetProjMatrix() const;

	DirectX::XMMATRIX GetModelMatrix(uint32_t index) const;

	uint32_t GetObjectCount() const {
		return OBJECT_COUNT;
	}

private:
	static constexpr uint32_t OBJECT_COUNT = 2;

	float m_elapsed = 0.0f;
	float m_aspect = 1.0f;
	
	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存
};