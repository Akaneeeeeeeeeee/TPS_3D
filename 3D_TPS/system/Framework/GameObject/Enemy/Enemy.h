#pragma once
#include "system/Framework/GameObject/Character/Character.h"

// 前方宣言
class Player;
class CharacterVirtualComponent;
class EnemyAIComponent;

/*
* @brief	敵クラス
* @detail	ゲーム内の敵キャラクターを表すクラス
* @remark	Characterクラスを継承。障害物を回避しながらの巡回とプレイヤー発見の基本的なAIを実装
* @auther	赤根　和樹
* @date		2025/11/16(コンポーネント化)
*/
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
	State   m_State = State::Patrol;
	Player* m_pPlayer = nullptr;

	// 視野パラメータ（今まで通り）
	float m_ViewAngle = 60.0f;
	float m_ViewDistance = 500.0f;

	// 「前の実装で使っていた巡回用情報」はここでは使わず、
	// コンポーネントに渡すための一時データとしてだけ使ってもOK
	Vector3 m_StartPos{};
	Vector3 m_EndPos{};

	// 便利のためにコンポーネントへのポインタを握っておく（なくても動く）
	CharacterVirtualComponent* m_CharComp = nullptr;
	EnemyAIComponent* m_AIComp = nullptr;

	bool IsFound = false;		// プレイヤーを発見したかどうか
};

