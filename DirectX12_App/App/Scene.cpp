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
}