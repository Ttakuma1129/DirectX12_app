#include "Camera.h"
#include <algorithm>

void Camera::Initialize(float aspect) {
	m_aspect = aspect;
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
	XMVECTOR up = XMVectorSet(0.0f, 0.1f, 0.0f, 0.0f);
	return XMMatrixLookAtLH(eye, target, up);
}

DirectX::XMMATRIX Camera::GetProjMatrix() const {
	float fov = DirectX::XMConvertToRadians(m_fov);
	return DirectX::XMMatrixPerspectiveFovLH(fov, m_aspect, 0.1f, 100.0f);
}