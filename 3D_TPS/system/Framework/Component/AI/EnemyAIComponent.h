#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"
#include <vector>
#include <array>

// 前方宣言
class PhysicsManager;
class CharacterVirtualComponent;
class Player;
class WeatherSystem;
class StaticMeshCollider;
class LightSystem;

namespace
{
    constexpr float DEG2RAD = PI / 180.0f;
	constexpr int SamplePointCount = 4; // 視線サンプリング点数
}

/*
* @brief    敵AIコンポーネント
* @detail   指定地点を巡回するように移動し、障害物を回避するAIコンポーネント
* @author   赤根　和樹
* @date     2025/11/16
*/
class EnemyAIComponent : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(EnemyAIComponent, IComponent)
    void Init(void) override;
    void Update(const float deltatime) override;
    void Uninit(void) override;

    void Attach(EngineServices& ctx) override;
    void Detach(void) override;

    // 聴覚入力
    void OnHeardSound(const Vector3& pos, float strength);

    enum State {
        Idle,		    // 待機状態
        Caution,        // 警戒状態（その場で振り向き＋待機）
        Patrol,		    // 巡回状態
        Investigate,    // 調査状態（音の方向へ移動）
        STATE_MAX,
    };

	State GetState(void) const { return m_State; }

    void SetWayPoints(const std::vector<Vector3>& waypoints) { m_WayPoints = waypoints; }
	const std::vector<Vector3>& GetWayPoints() const { return m_WayPoints; }
    void SetArriveRadius(float radius) { m_ArriveRadius = radius; }
    void SetRayLength(float length) { m_RayLength = length; }
    void SetAvoidWeight(float weight) { m_AvoidWeight = weight; }
    void SetEyeHeight(float height) { m_EyeHeight = height; }
    void SetTerrainCollider(StaticMeshCollider* col) { m_TerrainCol = col; }
    void SetViewParams(float angleDeg, float distance)
    {
        // 基準視界距離
        m_BaseViewDistance = distance;
        m_CurrentViewDistance = distance;  // 初期状態では一致させておく

        // 基準視野角（度 → ラジアン）
        m_BaseFOV = angleDeg * DEG2RAD;
        m_CurrentFOV = m_BaseFOV;
    }
    void SetPlayer(Player* player) { m_pPlayer = player; }

    // 今フレーム聞こえた音位置を 1 回だけ取り出す
    bool ConsumeHeardSoundPosition(Vector3& outPos);
    // Caution 中の「振り向きフェーズ」かどうか
    bool IsInCautionTurnPhase() const
    {
        return (m_State == State::Caution && m_CautionTurnTime < m_CautionTurnDuration);
    }

    // Player* がセットされているならプレイヤーを見る
    bool CanSeePlayer(void) const;

    bool IsFound(void) const { return m_IsFound; }

    static Vector3 LerpDir(const Vector3& a, const Vector3& b, float t)
    {
        Vector3 v = a * (1.0f - t) + b * t;
        if (v.LengthSquared() < 1e-6f) return b;
        v.Normalize();
        return v;
    }

    float GetSuspicion01() const { return m_Suspicion; } // デバッグ用

    bool IsInPatrolTurnPhase() const { return m_IsPatrolTurning; }
    bool IsPatrolTurnRight() const { return m_PatrolTurnRight; }

private:
    PhysicsManager* m_Physics = nullptr;
    CharacterVirtualComponent* m_Char = nullptr;
    Player* m_pPlayer = nullptr;
    WeatherSystem* m_Weather = nullptr;
    StaticMeshCollider* m_TerrainCol = nullptr;
    LightSystem* m_Light = nullptr;

    // ---------- 経路・視線などのベクトル系 ----------
    std::vector<Vector3> m_WayPoints;
    Vector3              m_LastHeardPosition = Vector3::Zero;   // 最後に聞こえた音の位置
    Vector3              m_ViewForward = Vector3::Forward;      // 身体とは独立した「視線用 forward」
    // Caution 用：視線の開始方向・目標方向
    Vector3              m_CautionStartViewDir = Vector3::Forward;
    Vector3              m_CautionTargetViewDir = Vector3::Forward;
    Vector3              m_LastMoveDir = Vector3::Forward;

    void UpdateStuck(float dt, const Vector3& desiredDir);
    void ResolveStuck(void);

    float HoldTimeByDistance(float dist) const;
    void  UpdateSuspicionFromSight(float dt);

	// 状態ごとの更新処理
    void UpdateIdle(const float deltatime);
    void UpdatePatrol(const float deltatime);
    void UpdateInvestigate(const float deltatime);
	void UpdateCaution(const float deltatime);

    // 移動計算用のヘルパ
	Vector3 ComputeAvoidDir(const Vector3& desired_dir);            // 簡易版
	Vector3 ComputeMoveDirToTarget(const Vector3& target);          // 目標地点への移動方向計算
	void FaceMoveDir(const Vector3& moveDir);   // キャラの向きを移動方向に合わせる
	bool FindLocalEscape(Vector3& outPos, const float maxRadius);   // 近くの障害物から逃げる位置を探す
    bool IsCapsuleFree(const Vector3& feetPos) const;


    // 視線更新
    void    UpdateSight(const float deltatime);
    Vector3 GetEyePosition(void) const;
	Vector3 GetViewForward(void) const { return m_ViewForward; }
	bool IsInViewCone(const Vector3& eyePos, const Vector3& targetPos) const;
	bool CanSeePoint(const Vector3& eyePos, const Vector3& targetPos) const;
    
    State m_State = State::Patrol;
	bool    m_HeardThisFrame = false;               // 今フレーム音を聞いたかどうか

    //壁回避モード
    bool m_IsAvoidingWall = false;
    float m_AvoidSideSign = 1.0f;   // +1 = 左方向、-1 = 右方向

	int   m_CurrentIndex = 0;       // 現在の巡回地点インデックス
	float m_ArriveRadius = 100.0f;   // 到着判定半径
	float m_RayLength = 800.0f;     // 障害物回避用のRay長さ
	float m_AvoidWeight = 1.5f;     // 障害物回避の重み付け
	float m_EyeHeight = 80.0f;      // Rayの発射位置（敵の目の高さ）

    // 調査状態用
    float m_InvestigateWaitTime = 2.0f; // 調査場所に到着後に何秒様子を見るか
	float m_InvestigateTimer = 0.0f;    // 調査中の経過時間

    // Caution（その場で振り向き＋待機）用
    float      m_CautionTurnDuration = 1.0f;  // 旋回にかける時間(秒)
    float      m_CautionWaitDuration = 3.0f;  // 振り向き後にその場で待つ時間

    float      m_CautionTurnTime = 0.0f;
    float      m_CautionWaitTime = 0.0f;
    Quaternion m_CautionStartRot{};
    Quaternion m_CautionTargetRot{};
    bool       m_CautionTurnRight = false;
    
	// 視野パラメータ
    // 視野パラメータ（「環境に依存しない基準値」と「環境込みの現在値」を分ける）
    float m_BaseViewDistance = 800.0f;              // 基準の視界距離
    float m_CurrentViewDistance = 0.0f;              // 環境を反映した視界距離

    float m_BaseFOV = 80.0f * DEG2RAD;    // 基準の視野角（ラジアン）
    float m_CurrentFOV = 60.0f * DEG2RAD;    // 環境を反映した視野角（ラジアン）
    bool m_HasLookedAtHeard = false;

    // ==== スタック検出用 ====
    Vector3 m_LastPosForStuck = Vector3::Zero;
    float   m_StuckTimer = 0.0f;
    bool    m_IsStuck = false;

    // 距離ベースのスタック判定用
    float   m_LastDistToTarget = 0.0f;
    bool    m_HasLastDistToTarget = false;

    // スタック解除試行回数（探索半径を広げるため）
    int     m_StuckResolveCount = 0;

    bool  m_IsFound = false;


	// ===== 不審度関連 =====
    // 不審度(0..1)
    float m_Suspicion = 0.0f;

    // 各点の「連続で見えていた秒数」
    std::array<float, SamplePointCount> m_SeenSec{};

    // 今フレーム「レイが1本でも通ったか」（後で音ブーストに使える）
    bool m_CanSeeAnyPointThisFrame = false;

    // ===== 調整パラメータ=====
    float m_SusGainPerSec = 0.9f;    // 見えてる時の基本増加
    float m_SusLosePerSec = 0.3f;    // 見えない時の減少

    // 「見え続けた」判定の必要秒数（近いほど短い）
    float m_HoldNearSec = 0.15f;
    float m_HoldFarSec = 1.00f;

    // 見えが途切れた時の減衰（0だと“厳密に連続”）
    float m_SeenDecayPerSec = 4.0f;  // 途切れたら素早く0へ

    // ライト倍率：1 + light01 * m_LightBoost
    float m_LightBoost = 1.5f;

    // 見えてる点数(0..4)による倍率（0点は0）
    std::array<float, 5> m_PointCountMul = { 0.0f, 0.7f, 1.0f, 1.6f, 2.3f };

    // 調査で向かうべき地点（音でも視覚でもここに入れる）
    Vector3 m_InvestigateTarget = Vector3::Zero;
    bool    m_HasInvestigateTarget = false;

    // 視覚で最後に見えた地点（失見後の調査に使う）
    Vector3 m_LastSeenPos = Vector3::Zero;
    bool    m_HasLastSeenPos = false;


    enum class AvoidMode : uint8_t { None, Steer, WallFollow };
    AvoidMode m_AvoidMode = AvoidMode::None;
    int   m_WallFollowSide = 0;       // +1:左沿い / -1:右沿い / 0:未決定
    float m_ClearTimer = 0.0f;        // 壁沿い解除用

	// ===== 巡回用 =====
    bool  m_IsPatrolTurning = false;
    bool  m_PatrolTurnRight = false;
    float m_PatrolTurnTime = 0.0f;
    float m_PatrolTurnDuration = 0.35f; // 調整
    float m_PatrolStartYaw = 0.0f;
    float m_PatrolTargetYaw = 0.0f;
    int   m_PatrolNextIndex = 0;

    float m_PatrolMoveAmount = 0.5f; // 巡回の入力量(歩き)
    float m_InvestigateMoveAmount = 0.80f; // 調査の入力量(速め)
};
