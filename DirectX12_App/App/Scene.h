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

	float& GetRotationSpeed() {
		return m_rotationSpeed;
	}

	float* GetCameraPos() {
		return m_cameraPos;
	}

	float& GetFov() {
		return m_fov;
	}

	float& GetModelScale() {
		return m_scale;
	}

	uint32_t GetObjectCount() const {
		return OBJECT_COUNT;
	}

	float* GetLightDir() {
		return m_lightDir;
	}

	float* GetLightColor() {
		return m_lightColor;
	}

	float* GetAmbientColor() {
		return m_ambientColor;
	}

private:
	static constexpr uint32_t OBJECT_COUNT = 2;

	float m_elapsed = 0.0f;
	float m_aspect = 1.0f;
	float m_rotationSpeed = 0.5f;
	float m_cameraPos[3] = { 0.0f,0.7f,-3.0f };
	float m_fov = 45.0f;
	float m_scale = 2.0f;

	// ライトパラメータ
	float m_lightDir[3] = { -0.5f, -1.0, 0.5f };
	float m_lightColor[3] = { 1.0f, 1.0f, 1.0f };
	float m_ambientColor[3] = { 0.15f, 0.15f, 0.15f };
	
	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存
};