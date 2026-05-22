#include "Player.h"
#include "../World/World.h"

void Player::Initialize(const DirectX::XMFLOAT3& spawnPos) {
	m_position = spawnPos;
	m_velocity = { 0, 0, 0 };
	m_onGround = false;
}

void Player::Update(float deltaTime, const World& world, bool forward, bool backward, bool left, bool right, bool jump, float yaw) {
	// FPSCameraのyawと同じ向きに前/右ベクトルを作る
	float forwardX = sinf(yaw);
	float forwardZ = cosf(yaw);
	float rightX = cosf(yaw);
	float rightZ = -sinf(yaw);

	float moveX = 0, moveZ = 0;
	if (forward) {
		moveX += forwardX;
		moveZ += forwardZ;
	}
	if (backward) {
		moveX -= forwardX;
		moveZ -= forwardZ;
	}
	if (right) {
		moveX += rightX;
		moveZ += rightZ;
	}
	if (left) {
		moveX -= rightX;
		moveZ -= rightZ;
	}

	// 斜め移動でも速度が一定になるように正規化する
	float length = sqrtf(moveX * moveX + moveZ * moveZ);
	if (length > 0.0f) {
		moveX /= length;
		moveZ /= length;
	}

	m_velocity.x = moveX * MOVE_SPEED;
	m_velocity.z = moveZ * MOVE_SPEED;

	// ジャンプ
	if (jump && m_onGround) {
		m_velocity.y = JUMP_SPEED;
		m_onGround = false;
	}

	// 重力
	m_velocity.y -= GRAVITY * deltaTime;
	// 落下速度の上限
	if (m_velocity.y < -50.0f) {
		m_velocity.y = -50.0f;
	}

	m_onGround = false;

	// 軸ごとに移動
	MoveAxis(0, m_velocity.x * deltaTime, world);
	MoveAxis(1, m_velocity.y * deltaTime, world);
	MoveAxis(2, m_velocity.z * deltaTime, world);

}

bool Player::IntersectsBlock(int bx, int by, int bz)const {
	DirectX::XMFLOAT3 mn, mx;
	GetAABB(mn, mx);
	return (mn.x< bx + 1.0f && mx.x > bx) &&
		   (mn.y< by + 1.0f && mx.y > by) &&
		   (mn.x< bz + 1.0f && mx.z > bz);
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
	int y1 = (int)floorf(mx.y - 1e-4f);
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