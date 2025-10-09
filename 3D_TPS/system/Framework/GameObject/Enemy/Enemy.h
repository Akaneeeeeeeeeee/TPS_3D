#pragma once
#include "system/Framework/GameObject/Character/Character.h"


class Enemy : public Character
{
public:
	Enemy() = default;
	Enemy(EngineContext& context, uint64_t id, const std::string& name = "", const Tag& tag = Tag::Enemy);
	~Enemy();

	void Init(void) override;
	void Update(uint64_t deltatime) override;
	void Draw(uint64_t deltatime) override;
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
};

