#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <cstdint>
#include <vector>
#include <string>

#include "../Engine/Resources/SceneObject.h"
#include "../Engine/Core/FPSCamera.h"
#include "Player/Player.h"

class Scene {
public:
	void Initialize(float aspectRatio);
	void Update();

	DirectX::XMMATRIX GetModelMatrix(uint32_t index) const;

	float& GetRotationSpeed() {
		return m_rotationSpeed;
	}

	float& GetModelScale() {
		return m_scale;
	}

	uint32_t GetObjectCount() const {
		return static_cast<uint32_t>(m_objects.size());
	}

	// Renderer用
	const FPSCamera& GetFPSCamera() const {
		return m_fpsCamera;
	}
	const Player& GetPlayer() const {
		return m_player;
	}
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
	float GetSapcIntensity() const {
		return m_specIntensity;
	}
	float GetSpecShiciness() const {
		return m_specShininess;
	}
	float GetShadowBias() const {
		return m_shadowBias;
	}
	const float* GetFogColor() const{
		return m_fogColor;
	}
	float GetFogStart() const{
		return m_fogStart;
	}
	float GetFogEnd() const{
		return m_fogEnd;
	}

	// ImGui用
	FPSCamera& GetFPSCamera() {
		return m_fpsCamera;
	}
	Player& GetPlayer() {
		return m_player;
	}
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
	float& GetSapcIntensity() {
		return m_specIntensity;
	}
	float& GetSpecShiciness() {
		return m_specShininess;
	}
	float& GetShadowBias() {
		return m_shadowBias;
	}
	float* GetFogColor() {
		return m_fogColor;
	}
	float& GetFogStart() {
		return m_fogStart;
	}
	float& GetFogEnd() {
		return m_fogEnd;
	}

private:
	FPSCamera m_fpsCamera;
	Player m_player;

	std::vector<SceneObject> m_objects;

	float m_elapsed = 0.0f;
	float m_rotationSpeed = 0.0f;
	float m_scale = 2.0f;

	// ライトパラメータ
	float m_lightDir[3] = { -0.5f, -1.0, 0.5f };
	float m_lightColor[3] = { 1.0f, 1.0f, 1.0f };
	float m_ambientColor[3] = { 0.25f, 0.25f, 0.25f };
	float m_specIntensity = 0.5f; // スペキュラー強度
	float m_specShininess = 32.0f; // ハイライトの鋭さ

	float m_shadowBias = 0.0003f;

	// フォグ
	float m_fogColor[3] = { 0.87f,0.91f,0.93 };
	float m_fogStart = 40.0f; // フォグの開始位置
	float m_fogEnd = 64.0f; // 終了位置

	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存
};