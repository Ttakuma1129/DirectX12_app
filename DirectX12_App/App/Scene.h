#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <cstdint>

class Scene {
public:
	void Initialize();
	void Update();

	DirectX::XMMATRIX GetViewMatrix() const {

	}

	DirectX::XMMATRIX GetProjMatrix() const {

	}

	uint32_t GetObjectCount() const {
		return OBJECT_COUNT;
	}

private:
	static constexpr uint32_t OBJECT_COUNT = 2;

	float m_elapsed = 0.0f;
	float m_aspect = 16.0f / 9.0f;
	
	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存
};