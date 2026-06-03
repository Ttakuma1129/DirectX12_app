#pragma once
#include <DirectXMath.h>

class World;

class Player {
public:
	void Initialize(const DirectX::XMFLOAT3& spawnPos);

	// 入力によって位置と速度を更新
	void Update(float deltaTime, const World& world, bool forward, bool backward, bool left, bool right, bool jump, float yaw);

	// 指定したブロックセルがプレイヤーの当たり判定と重なるか
	bool IntersectsBlock(int bx, int by, int bz)const;

	DirectX::XMFLOAT3 GetPosition() const {
		return m_position;
	}

	void SetPosition(const DirectX::XMFLOAT3& pos) {
		m_position = pos;
	}

	// 目(カメラ)の位置
	DirectX::XMFLOAT3 GetEyePosition() const {
		return { m_position.x + 0.5f * WIDTH,
				 m_position.y + EYE_HEIGHT,
				 m_position.z + 0.5f * WIDTH };
	}

	bool IsOnGround() const {
		return m_onGround;
	}

private:
	// プレイヤーのサイズ・物理定数
	static constexpr float WIDTH = 0.6f;
	static constexpr float HEIGHT = 1.8f;
	static constexpr float EYE_HEIGHT = 1.6f;
	static constexpr float MOVE_SPEED = 5.0f;
	static constexpr float JUMP_SPEED = 9.0f;
	static constexpr float GRAVITY = 28.0f;

	DirectX::XMFLOAT3 m_position = { 0, 0, 0 }; // 足元の位置
	DirectX::XMFLOAT3 m_velocity = { 0, 0, 0 };

	bool m_onGround = false; // 地面に触れているかどうか(trueで地面に触れている)

	// AABBを取得
	void GetAABB(DirectX::XMFLOAT3& mn, DirectX::XMFLOAT3& mx) const;

	// 指定したワールド領域にブロックがあるかどうか
	bool CollidesWithWorld(const World& world, const DirectX::XMFLOAT3& mn, const DirectX::XMFLOAT3& mx) const;

	// 軸ごとに移動を試みて衝突したら停止
	void MoveAxis(int axis, float delta, const World& world);
};