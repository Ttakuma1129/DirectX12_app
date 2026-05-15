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

}