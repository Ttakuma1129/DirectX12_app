#include <algorithm>
#include "Camera.h"
void Camera::Initialize(float aspect) {
	m_aspect = aspect;
}

void Camera::Rotate(float dx, float dy) {
	m_yaw += dx * m_rotateSensitivity;
	m_pitch += dy * m_rotateSensitivity;

	const float limit = DirectX::XM_PIDIV2 - 0.01f;
	m_pitch = std::clamp(m_pitch, -limit, limit);
}

void Camera::Zoom(float delta) {
	m_distance -= delta * m_zoomSensitivity;
	m_distance = std::clamp(m_distance, 0.5f, 50.0f);
}

void Camera::Pan(float dx, float dy) {
	using namespace DirectX;

	float cosPitch = cosf(m_pitch);
	float rightX = cosf(m_yaw);
	float rightZ = -sinf(m_yaw);
	float upX = -sinf(m_pitch) * sinf(m_yaw);
	float upY = cosPitch;
	float upZ = -sinf(m_pitch) * cosf(m_yaw);

	float scale = m_distance * m_pansensitivity;
	m_target[0] -= (rightX * dx + upX * dy) * scale;
	m_target[1] -= (upY * dy) * scale;
	m_target[2] -= (rightZ * dx + upZ * dy) * scale;
}

DirectX::XMFLOAT3 Camera::GetPosition() const {
	float cosPitch = cosf(m_pitch);
	return{
		m_target[0] + m_distance * cosPitch * sinf(m_yaw),
		m_target[1] + m_distance * sinf(m_pitch),
		m_target[2] + m_distance * cosPitch * cosf(m_yaw)
	};
}

DirectX::XMMATRIX Camera::GetViewMatrix() const {
	using namespace DirectX;
	XMFLOAT3 pos = GetPosition();
	XMVECTOR eye = XMLoadFloat3(&pos);
	XMVECTOR target = XMVectorSet(m_target[0], m_target[1], m_target[2], 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	return XMMatrixLookAtLH(eye, target, up);
}

DirectX::XMMATRIX Camera::GetProjMatrix() const {
	float fov = DirectX::XMConvertToRadians(m_fov);
	return DirectX::XMMatrixPerspectiveFovLH(fov, m_aspect, 0.1f, 100.0f);
}