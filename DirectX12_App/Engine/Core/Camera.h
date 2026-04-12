#pragma once
#include <DirectXMath.h>

class Camera {
public:
	void Initialize(float aspect);

	// マウス操作
	void Rotate(float dx, float dy); // 左クリック+ドラッグ:回転
	void Zoom(float delta); // ホイール:ズーム
	void Pan(float dx, float dy); // ホイールクリック+ドラッグ:平行移動

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjMatrix() const;

	DirectX::XMFLOAT3 GetPosition() const;

	float& GetFov() {
		return m_fov;
	}
	float GetAspect() const {
		return m_aspect;
	}
private:
	float m_yaw = 0.0f;	// 水平角
	float m_pitch = 0.2f; // 垂直角
	float m_distance = 3.0f; // ターゲットからの距離

	float m_target[3] = { 0.0f, 0.7f,0.0f }; // 注視点

	//投影
	float m_fov = 45.0f; // 視野
	float m_aspect = 1.0f;

	// 操作感度
	float m_rotateSensitivity = 0.005f;
	float m_zoomSensitivity = 0.2f;
	float m_pansensitivity = 0.0005f;
};