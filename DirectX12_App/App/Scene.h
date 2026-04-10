#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <cstdint>
#include <vector>
#include <string>

#include "../Engine/Resources/SceneObject.h"

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

	float& GetFov() {
		return m_fov;
	}

	float& GetModelScale() {
		return m_scale;
	}

	uint32_t GetObjectCount() const {
		return static_cast<uint32_t>(m_objects.size());
	}

	// Renderer用
	const std::vector<SceneObject>& GetObjects() const {
		return m_objects;
	}
	const float* GetLightDir() const {
		return m_lightDir;
	}
	const float* GetLightColor() const {
		return m_lightColor;
	}
	const float* GetAmbientColor() const {
		return m_ambientColor;
	}
	const float* GetCameraPos() const {
		return m_cameraPos;
	}
	float GetSapcIntensity() const {
		return m_specIntensity;
	}
	float GetSpecShiciness() const {
		return m_specShininess;
	}

	// ImGui用
	std::vector<SceneObject>& GetObjects() {
		return m_objects;
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
	float* GetCameraPos() {
		return m_cameraPos;
	}
	float& GetSapcIntensity() {
		return m_specIntensity;
	}
	float& GetSpecShiciness() {
		return m_specShininess;
	}

private:
	std::vector<SceneObject> m_objects;

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
	float m_specIntensity = 0.5f; // スペキュラー強度
	float m_specShininess = 32.0f; // ハイライトの鋭さ
	
	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存
};