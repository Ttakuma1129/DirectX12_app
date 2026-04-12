#include "Scene.h"

void Scene::Initialize(float aspectRatio) {
	m_camera.Initialize(aspectRatio);

	// 開始時間を記録
	m_startTime = std::chrono::high_resolution_clock::now();

	// 起動時にオブジェクトを2つ設置
	SceneObject obj1;
	strcpy_s(obj1.name, "Sword");
	obj1.modelPath = "App/Models/sword.obj";
	obj1.meshIndex = 0;
	obj1.position[0] = -1.5f;
	obj1.scale = 0.1f;
	m_objects.push_back(obj1);

	SceneObject obj2;
	strcpy_s(obj2.name, "Sphere");
	obj2.modelPath = "App/Models/sphere.obj";
	obj2.meshIndex = 0;
	obj2.position[0] = 1.5f;
	obj2.scale = 0.1f;
	m_objects.push_back(obj2);
}

void Scene::Update() {
	// 経過時間を秒で取得
	auto now = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration<float>(now - m_startTime).count();
	m_elapsed = elapsed;
}

DirectX::XMMATRIX Scene::GetModelMatrix(uint32_t index) const {
	using namespace DirectX;
	if (index >= m_objects.size()) {
		return XMMatrixIdentity();
	}

	const auto& obj = m_objects[index];

	XMMATRIX S = XMMatrixScaling(obj.scale, obj.scale, obj.scale);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(obj.rotation[0]),
		XMConvertToRadians(obj.rotation[1]) + m_elapsed * XM_2PI * m_rotationSpeed,
		XMConvertToRadians(obj.rotation[2])
	);
	XMMATRIX T = XMMatrixTranslation(obj.position[0], obj.position[1], obj.position[2]);
	
	return S * R * T;
}