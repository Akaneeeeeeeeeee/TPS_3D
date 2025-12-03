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
	Enemy(ComponentFactory* factory, const uint64_t id, const std::string& name = "", const Tag& tag = Tag::Enemy,
		Player* player = nullptr,
		const Transform& transform = Transform::One());
	~Enemy();

	void Init(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	void SetPlayer(Player* player) { m_pPlayer = player; }
	bool CanSeePlayer(const Vector3& playerPos) const;
	void DebugImGui(void);
private:
	Player* m_pPlayer = nullptr;

	// ==== 驚きターン用 ====
	enum class FacingState
	{
		Normal,
		SurprisedTurn,
	};

	FacingState m_FacingState = FacingState::Normal;

	bool       m_TurnRight = false; // 右向きアニメかどうか
	bool TryStartSurpriseTurn(const Vector3& soundPos);

	// 「前の実装で使っていた巡回用情報」はここでは使わず、
	// コンポーネントに渡すための一時データとしてだけ使ってもOK
	Vector3 m_StartPos{};
	Vector3 m_EndPos{};

	// 便利のためにコンポーネントへのポインタを握っておく（なくても動く）
	CharacterVirtualComponent* m_CharComp = nullptr;
	EnemyAIComponent* m_AIComp = nullptr;

	// すでにゲームオーバー処理を走らせたかどうか
	bool m_GameOverTriggered = false;
};

