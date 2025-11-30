#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"

// 前方宣言
class PhysicsManager;
class CharacterVirtualComponent;
class Player;

/*
* @brief    敵AIコンポーネント
* @detail   指定地点を巡回するように移動し、障害物を回避するAIコンポーネント
* @author   赤根　和樹
* @date     2025/11/16
*/
class EnemyAIComponent : public IComponent
{
public:
    void Init(void) override;
    void Update(const float deltatime) override;
    void Uninit(void) override;

    void Attach(EngineContext& ctx) override;
    void Detach(void) override;

    // 聴覚入力
    void OnHeardSound(const Vector3& pos, float strength);

    enum State {
        Idle,		    // 待機状態
		Caution,        // 警戒状態
        Patrol,		    // 巡回状態
        Investigate,    // 調査状態
        Chase,		    // 追跡状態
        STATE_MAX,
    };

    void SetWayPoints(const std::vector<Vector3>& waypoints) { m_WayPoints = waypoints; }
    void SetArriveRadius(float radius) { m_ArriveRadius = radius; }
    void SetRayLength(float length) { m_RayLength = length; }
    void SetAvoidWeight(float weight) { m_AvoidWeight = weight; }
    void SetEyeHeight(float height) { m_EyeHeight = height; }
    void SetViewParams(float angleDeg, float distance)
    {
        m_ViewAngle = angleDeg;
        m_ViewDistance = distance;
    }
    void SetPlayer(Player* player) { m_pPlayer = player; }

    // 任意ターゲット位置を見る
    bool CanSeeTarget(const Vector3& targetPos) const;
    // Player* がセットされているならプレイヤーを見る
    bool CanSeePlayer() const;

    bool IsFound() const { return m_IsFound; }

private:
    void UpdateIdle(const float deltatime);     // 待機状態の更新(今後実装)
    void UpdatePatrol(const float deltatime);
    void UpdateInvestigate(const float deltatime);
    void UpdateChase(const float deltatime);    // 追跡状態の更新(今後実装)

    // 移動計算用のヘルパ
	Vector3 ComputeAvoidDir(const Vector3& desired_dir);        // 簡易版
	Vector3 ComputeMoveDirToTarget(const Vector3& target);      // 目標地点への移動方向計算
    void FaceMoveDir(const Vector3& moveDir);

    // 視線更新
    void    UpdateSight(const float deltatime);
    Vector3 GetEyePosition() const;
    Vector3 GetForwardFromOwnerRotation() const;

    PhysicsManager* m_Physics = nullptr;
    CharacterVirtualComponent* m_Char = nullptr;
    Player* m_pPlayer = nullptr;

    State m_State = State::Patrol;
    Vector3 m_LastHeardPosition = Vector3::Zero;

    //壁回避モード
    bool m_IsAvoidingWall = false;
    float m_AvoidSideSign = 1.0f;   // +1 = 左方向、-1 = 右方向

    std::vector<Vector3> m_WayPoints;
	int   m_CurrentIndex = 0;       // 現在の巡回地点インデックス
	float m_ArriveRadius = 75.0f;   // 到着判定半径
	float m_RayLength = 300.0f;     // 障害物回避用のRay長さ
	float m_AvoidWeight = 1.5f;     // 障害物回避の重み付け
	float m_EyeHeight = 80.0f;      // Rayの発射位置（敵の目の高さ）

    // 調査状態用
    float m_InvestigateWaitTime = 2.0f; // 調査場所に到着後に何秒様子を見るか
	float m_InvestigateTimer = 0.0f;    // 調査中の経過時間

    float m_ViewAngle = 60.0f;
    float m_ViewDistance = 500.0f;

    bool  m_IsFound = false;

    // 視線チェックの頻度
    float m_SightCheckInterval = 0.1f; // 0.1秒ごと
    float m_SightCheckTimer = 0.0f;
};
