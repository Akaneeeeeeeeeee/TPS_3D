#pragma once
#include "system/Framework/GameObject/Character/Character.h"
#include "Framework/Component/AI/EnemyAIComponent.h"

// 前方宣言
class Player;
class CharacterVirtualComponent;
class EnemyAIComponent;
class Terrain;
class StaticMeshCollider;
class EnemyHeadIconComponent;

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

	void Awake(void) override;
	void Start(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	void SetPlayer(Player* player) { m_pPlayer = player; }
	void SetTerrain(Terrain* terrain) { m_pTerrain = terrain; }
	void SetWayPoints(const Vector3& start, const Vector3& end)
	{
		m_StartPos = start;
		m_EndPos = end;
	}
	void SetWayPoints(const std::vector<Vector3>& points);

	bool CanSeePlayer(const Vector3& playerPos) const;
	bool IsGameOverTriggered(void) const { return m_GameOverTriggered; }
	void DebugImGui(void);
private:
	void InitAnimation(void);
	void InitPatrolPoints(RandomEngine& rng);
	void InitComponents(void);

	bool TryStartSurpriseTurn(const Vector3& soundPos);
	void OnFoundPlayer(void);

	Player* m_pPlayer = nullptr;
	Terrain* m_pTerrain = nullptr;
	StaticMeshCollider* m_pTerrainCollider;
	CharacterVirtualComponent* m_CharComp = nullptr;
	EnemyAIComponent* m_AIComp = nullptr;
	EnemyHeadIconComponent* m_HeadIcon = nullptr;

	bool       m_TurnRight = false; // 右向きアニメかどうか
	// 巡回点
	Vector3 m_StartPos{};
	Vector3 m_EndPos{};

	// ==== 驚きターン用 ====
	enum class FacingState
	{
		Normal,
		SurprisedTurn,
	};
	FacingState m_FacingState = FacingState::Normal;
	EnemyAIComponent::State m_PrevAIState;

	// すでにゲームオーバー処理を走らせたかどうか
	bool        m_GameOverTriggered = false;
};

