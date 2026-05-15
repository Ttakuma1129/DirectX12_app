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

}

void Player::MoveAxis(int axis, float delta, const World& world) {

}