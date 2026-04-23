#pragma once
#include <DirectXMath.h>

class FPSCamera {
public:
	void Initialize(float aspect);

	// 入力によって位置と向きを変える
	void Update(float deltaTime, bool forward, bool backward, bool left, bool right, bool up, bool down);
	
	// マウス入力 : 視点の向きを回転
	void Rotate(float dx, float dy);

	// 行列取得
	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjMatrix() const;
	DirectX::XMFLOAT3 GetPosition() const {
		return m_position;
	}

	float& GetFov() {
		return m_fov;
	}
	float GetAspect() const {
		return m_aspect;
	}

private:
	DirectX::XMFLOAT3 m_position = { 8.0f,20.0f,8.0f }; // ワールド座標
	float m_yaw = 0.0f; // 水平角
	float m_pitch = 0.0f; // 垂直角

	float m_fov = 60.0f;
	float m_aspect = 1.0f;

	float m_moveSpeed = 10.0f;
	float m_mouseSensitivity = 0.002f;
};