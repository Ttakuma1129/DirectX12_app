#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <cstdint>
#include <vector>
#include <string>

// オブジェクト1つ分のデータ
struct SceneObject {
	std::string modelPath;	// モデルのファイルパス
	uint32_t meshIndex = 0; // Rendererで使うメッシュ番号
	float position[3] = { 0,0,0 }; //オブジェクトの位置
	float rotation[3] = { 0,0,0 }; // オブジェクトの回転(ラジアン)
	float scale = 1.0f;
	char name[32] = "Object"; // ImGuiで表示する名前
};

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
		return OBJECT_COUNT;
	}

	// Renderer用
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
	float m_specIntensity = 0.5f; // スペキュラー強度
	float m_specShininess = 32.0f; // ハイライトの鋭さ
	
	std::chrono::high_resolution_clock::time_point m_startTime; // フレームの開始時間を保存
};