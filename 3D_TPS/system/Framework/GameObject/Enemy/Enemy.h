#pragma once
#include "system/Framework/GameObject/Character/Character.h"

// 前方宣言
class Player;


class Enemy : public Character
{
public:
	Enemy() = default;
	Enemy(EngineContext& context, const uint64_t id, const std::string& name = "", const Tag& tag = Tag::Enemy, 
		Player* player = nullptr,
		const Transform& transform = Transform::One());
	~Enemy();

	enum State {
		Idle,		// 待機状態
		Patrol,		// 巡回状態
		Chase,		// 追跡状態
		STATE_MAX,
	};

	void Init(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	bool CanSeePlayer(const Vector3& playerPos) const;

private:
	// 視野
	float m_ViewAngle = 60.0f;
	// 最大視野距離
	float m_ViewDistance = 500.0f;

	// 移動用変数
	Vector3 m_StartPos{};
	Vector3 m_EndPos{};
	Vector3 m_TargetPos{};
	std::vector<Vector3> m_PatrolPoints;	// 巡回ポイント
	bool m_GoingToEnd = true; // true: Start→End, false: End→Start
	bool IsFound = false;		// プレイヤーを発見したかどうか

	Player* m_pPlayer = nullptr;	// プレイヤーへのポインタ
};

