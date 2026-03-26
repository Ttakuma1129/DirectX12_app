#include "Scene.h"

void Scene::Initialize(float aspectRatio) {
	m_aspect = aspectRatio;
	// 開始時間を記録
	m_startTime = std::chrono::high_resolution_clock::now();
}

void Scene::Update() {
	// 経過時間を秒で取得
	auto now = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration<float>(now - m_startTime).count();
	m_elapsed = elapsed;
}

DirectX::XMMATRIX Scene::GetViewMatrix()const {
	using namespace DirectX;
	// View行列 (カメラの設定)
	XMVECTOR eye = XMVectorSet(0.0f, 0.7f, -3.0f, 0.0f); // カメラ位置
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f); // 注視点
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向
	return XMMatrixLookAtLH(eye, target, up); // View行列
}

DirectX::XMMATRIX Scene::GetProjMatrix() const {
	using namespace DirectX;
	// Projection行列　(透視投影)
	float fov = XMConvertToRadians(45.0f); // 視野角
	return XMMatrixPerspectiveFovLH(fov, m_aspect, 0.1f, 100.0f);
}

DirectX::XMMATRIX Scene::GetModelMatrix(uint32_t index) const {
	using namespace DirectX;
	if (index == 0) {
		return XMMatrixRotationY(m_elapsed * XM_2PI * 0.5f) * XMMatrixTranslation(-1.5f, 0.0f, 0.0f);

	}
	else {
		return XMMatrixRotationY(-m_elapsed * XM_2PI * 0.5f) * XMMatrixTranslation(1.5f, 0.0f, 0.0f);

	}
}