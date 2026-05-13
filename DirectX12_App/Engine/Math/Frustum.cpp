#include "Frustum.h"

void Frustum::ExtractFormMatrix(const DirectX::XMMATRIX& viewProj) {
	using namespace DirectX;

	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, viewProj);

	m_planes[LEFT].x = m._14 + m._11;
	m_planes[LEFT].y = m._24 + m._21;
	m_planes[LEFT].z = m._34 + m._31;
	m_planes[LEFT].w = m._44 + m._41;

	m_planes[RIGHT].x = m._14 - m._11;
	m_planes[RIGHT].y = m._24 - m._21;
	m_planes[RIGHT].z = m._34 - m._31;
	m_planes[RIGHT].w = m._44 - m._41;

	m_planes[BOTTOM].x = m._14 + m._12;
	m_planes[BOTTOM].y = m._24 + m._22;
	m_planes[BOTTOM].z = m._34 + m._32;
	m_planes[BOTTOM].w = m._44 + m._42;

	m_planes[TOP].x = m._14 - m._12;
	m_planes[TOP].y = m._24 - m._22;
	m_planes[TOP].z = m._34 - m._32;
	m_planes[TOP].w = m._44 - m._42;

	m_planes[NEAR_].x = m._13;
	m_planes[NEAR_].y = m._23;
	m_planes[NEAR_].z = m._33;
	m_planes[NEAR_].w = m._43;

	m_planes[FAR_].x = m._14 - m._13;
	m_planes[FAR_].y = m._24 - m._23;
	m_planes[FAR_].z = m._34 - m._33;
	m_planes[FAR_].w = m._44 - m._43;
	
	// 各平面を正規化
	for (int i = 0; i < 6;++i) {
		float len = sqrtf(m_planes[i].x * m_planes[i].x + m_planes[i].y * m_planes[i].y + m_planes[i].z * m_planes[i].z);
		if (len > 0.0f) {
			m_planes[i].x /= len;
			m_planes[i].y /= len;
			m_planes[i].z /= len;
			m_planes[i].w /= len;
		}
	}
}

bool Frustum::IntersectsAABB(const DirectX::XMFLOAT3& mn, const DirectX::XMFLOAT3& mx) const {
	// 6平面それぞれに対して positive-vertex 判定
	for (int i = 0;i < 6;++i) {
		const DirectX::XMFLOAT4& p = m_planes[i];

		// 平面の法線方向に最も遠い頂点を選択
		// 法線成分が 正→max 負→minを選択
		float px = (p.x >= 0.0f) ? mx.x : mn.x;
		float py = (p.y >= 0.0f) ? mx.y : mn.y;
		float pz = (p.z >= 0.0f) ? mx.z : mn.z;

		float distance = p.x * px + p.y * py + p.z * pz + p.w;

		// 0未満(平面の外側)ならカリング
		if (distance < 0.0f) {
			return false;
		}
	}
	return true;
}