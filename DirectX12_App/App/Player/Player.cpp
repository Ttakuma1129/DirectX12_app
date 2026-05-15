#include "Player.h"
#include "../World/World.h"

void Player::Initialize(const DirectX::XMFLOAT3& spawnPos) {

}

void Player::Update(float deltaTime, const World& world, bool forward, bool backward, bool left, bool right, bool jump, float yaw) {

}

void Player::GetAABB(DirectX::XMFLOAT3& mn, DirectX::XMFLOAT3& mx) const {

}

bool Player::CollidesWithWorld(const World& world, const DirectX::XMFLOAT3& mn, const DirectX::XMFLOAT3& mx) const {

}

void Player::MoveAxis(int axis, float delta, const World& world) {

}