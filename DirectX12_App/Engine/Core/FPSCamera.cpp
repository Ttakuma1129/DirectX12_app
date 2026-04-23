#include "FPSCamera.h"

void FPSCamera::Initialize(float aspect) {
	m_aspect = aspect;
}

void FPSCamera::Update(float deltaTime, bool forward, bool backward, bool left, bool right, bool up, bool down) {
	using namespace DirectX;

	// 視線の前方ベクトルと右方向ベクトルを計算

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
	 return{ -forward.z, 0.0f, forward.x };
 }