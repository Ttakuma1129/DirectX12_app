#pragma once
#include <DirectXMath.h>

class Frustum {
public:
	// VP行列から6面を抽出
	void ExtractFormMatrix(const DirectX::XMMATRIX& viewProj);

	// AABBがフラスタムの内側または交差しているか
	bool IntersectsAABB(const DirectX::XMFLOAT3& mn, const DirectX::XMFLOAT3& mx) const;

private:
	DirectX::XMFLOAT4 m_planes[6];

	enum Plane {
		LEFT = 0,
		RIGHT,
		BOTTOM,
		TOP,
		NEAR_,
		FAR_
	};
};