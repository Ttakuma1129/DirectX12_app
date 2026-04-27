#include <algorithm>
#include "FPSCamera.h"

void FPSCamera::Initialize(float aspect) {
	m_aspect = aspect;
}

void FPSCamera::Update(float deltaTime, bool forward, bool backward, bool left, bool right, bool up, bool down) {
	using namespace DirectX;

	// 視線の前方ベクトルと右方向ベクトルを取得
	XMFLOAT3 fwd = GetForward();
	XMFLOAT3 rt = GetRight();

	// 入力によって位置を更新
	if (forward) {
		m_position.x += fwd.x * m_moveSpeed * deltaTime;
		m_position.y += fwd.y * m_moveSpeed * deltaTime;
		m_position.z += fwd.z * m_moveSpeed * deltaTime;
	}
	if (backward) {
		m_position.x -= fwd.x * m_moveSpeed * deltaTime;
		m_position.y -= fwd.y * m_moveSpeed * deltaTime;
		m_position.z -= fwd.z * m_moveSpeed * deltaTime;
	}
	if (right) {
		m_position.x += rt.x * m_moveSpeed * deltaTime;
		m_position.y += rt.y * m_moveSpeed * deltaTime;
		m_position.z += rt.z * m_moveSpeed * deltaTime;
	}
	if (left) {
		m_position.x -= rt.x * m_moveSpeed * deltaTime;
		m_position.y -= rt.y * m_moveSpeed * deltaTime;
		m_position.z -= rt.z * m_moveSpeed * deltaTime;
	}
	if (up) {
		m_position.y += m_moveSpeed * deltaTime;
	}
	if (down) {
		m_position.y -= m_moveSpeed * deltaTime;
	}
}

void FPSCamera::Rotate(float dx, float dy) {
	m_yaw += dx * m_mouseSensitivity;
	m_pitch -= dy * m_mouseSensitivity;

	const float limit = DirectX::XM_PIDIV2 - 0.01f;
	m_pitch = std::clamp(m_pitch, -limit, limit);
}

DirectX::XMMATRIX FPSCamera::GetViewMatrix() const {
	using namespace DirectX;
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMFLOAT3 forward3 = GetForward();
	XMVECTOR forward = XMLoadFloat3(&forward3);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	return XMMatrixLookToLH(pos, forward, up);
}

DirectX::XMMATRIX FPSCamera::GetProjMatrix() const {
	float fov = DirectX::XMConvertToRadians(m_fov);
	return DirectX::XMMatrixPerspectiveFovLH(fov, m_aspect, 0.1f, 100.0f);
}

 DirectX::XMFLOAT3 FPSCamera::GetForward() const {
	 DirectX::XMFLOAT3 forward;
	 forward.x = cosf(m_pitch) * sinf(m_yaw);
	 forward.y = sinf(m_pitch);
	 forward.z = cosf(m_pitch) * cosf(m_yaw);
	 return forward;
}

 DirectX::XMFLOAT3 FPSCamera::GetRight() const {
	 DirectX::XMFLOAT3 forward = GetForward();
	 return{ forward.z, 0.0f, -forward.x };
 }