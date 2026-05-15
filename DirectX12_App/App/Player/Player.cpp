#include "Player.h"
#include "../World/World.h"

void Player::Initialize(const DirectX::XMFLOAT3& spawnPos) {
	m_position = spawnPos;
	m_velocity = { 0, 0, 0 };
	m_onGround = false;
}

void Player::Update(float deltaTime, const World& world, bool forward, bool backward, bool left, bool right, bool jump, float yaw) {

}

void Player::GetAABB(DirectX::XMFLOAT3& mn, DirectX::XMFLOAT3& mx) const {
	mn = { m_position.x, m_position.y, m_position.z };
	mx = { m_position.x + WIDTH,
		   m_position.y + HEIGHT,
		   m_position.z + WIDTH };
}

bool Player::CollidesWithWorld(const World& world, const DirectX::XMFLOAT3& mn, const DirectX::XMFLOAT3& mx) const {
	// AABBが触れている全てのブロック座標を走査
	int x0 = (int)floorf(mn.x);
	int x1 = (int)floorf(mx.x - 1e-4f);
	int y0 = (int)floorf(mn.y);
	int y1 = (int)floorf(mn.y - 1e-4f);
	int z0 = (int)floorf(mn.z);
	int z1 = (int)floorf(mx.z - 1e-4f);

	for (int x = x0; x <= x1; ++x) {
		for (int y = y0; y <= y1; ++y) {
			for (int z = z0;z <= z1;++z) {
				if (world.IsSolid(x, y, z)) {
					return true;
				}
			}
		}
	}
	return false;
}

void Player::MoveAxis(int axis, float delta, const World& world) {
	if (delta == 0.0f) {
		return;
	}

	// 仮で動かす
	float& comp = (axis == 0) ? m_position.x : (axis == 1) ? m_position.y : m_position.z;
	float original = comp;
	comp += delta;

	// AABBで衝突判定
	DirectX::XMFLOAT3 mn, mx;
	GetAABB(mn, mx);
	if (CollidesWithWorld(world, mn, mx)) {
		// 衝突 → 仮に動かした分を戻しブロック境界までつめる
		comp = original;

		// Y軸で下方向に衝突 → 地面と接している
		if (axis == 1 && delta < 0.0f) {
			m_onGround = true;
		}
		// 衝突した軸の速度を無くす
		if (axis == 0) {
			m_velocity.x = 0.0f;
		}
		if (axis == 1) {
			m_velocity.y = 0.0f;
		}
		if (axis == 2) {
			m_velocity.z = 0.0f;
		}
	}
}