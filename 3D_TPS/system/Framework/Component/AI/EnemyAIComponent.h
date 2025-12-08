#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"

// 前方宣言
class PhysicsManager;
class CharacterVirtualComponent;
class Player;
class WeatherSystem;

namespace
{
    constexpr float DEG2RAD = PI / 180.0f;
}

/*
* @brief    敵AIコンポーネント
* @detail   指定地点を巡回するように移動し、障害物を回避するAIコンポーネント
* @author   赤根　和樹２４８
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
        Caution,        // 警戒状態（その場で振り向き＋待機）
        Patrol,		    // 巡回状態
        Investigate,    // 調査状態（音の方向へ移動）
        Chase,		    // 追跡状態
        STATE_MAX,
    };

	State GetState(void) const { return m_State; }

    void SetWayPoints(const std::vector<Vector3>& waypoints) { m_WayPoints = waypoints; }
    void SetArriveRadius(float radius) { m_ArriveRadius = radius; }
    void SetRayLength(float length) { m_RayLength = length; }
    void SetAvoidWeight(float weight) { m_AvoidWeight = weight; }
    void SetEyeHeight(float height) { m_EyeHeight = height; }
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


private:
    PhysicsManager* m_Physics = nullptr;
    CharacterVirtualComponent* m_Char = nullptr;
    Player* m_pPlayer = nullptr;
    WeatherSystem* m_Weather = nullptr;

    // ---------- 経路・視線などのベクトル系 ----------
    std::vector<Vector3> m_WayPoints;
    Vector3              m_LastHeardPosition = Vector3::Zero;   // 最後に聞こえた音の位置
    Vector3              m_ViewForward = Vector3::Forward;      // 身体とは独立した「視線用 forward」
    // Caution 用：視線の開始方向・目標方向
    Vector3              m_CautionStartViewDir = Vector3::Forward;
    Vector3              m_CautionTargetViewDir = Vector3::Forward;
    Vector3              m_LastMoveDir = Vector3::Forward;

    // スタック検出用
    Vector3 m_LastPosForStuck = Vector3::Zero;
    float   m_StuckTimer = 0.0f;
    bool    m_IsStuck = false;

    void UpdateStuck(float dt, const Vector3& desiredDir);
    void ResolveStuck();

	// 状態ごとの更新処理
    void UpdateIdle(const float deltatime);
    void UpdatePatrol(const float deltatime);
    void UpdateInvestigate(const float deltatime);
    void UpdateChase(const float deltatime);
	void UpdateCaution(const float deltatime);

    // 移動計算用のヘルパ
	Vector3 ComputeAvoidDir(const Vector3& desired_dir);        // 簡易版
	Vector3 ComputeMoveDirToTarget(const Vector3& target);      // 目標地点への移動方向計算
    void FaceMoveDir(const Vector3& moveDir);

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
	float m_ArriveRadius = 75.0f;   // 到着判定半径
	float m_RayLength = 1000.0f;     // 障害物回避用のRay長さ
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
    float m_BaseViewDistance = 1000.0f;              // 基準の視界距離
    float m_CurrentViewDistance = 0.0f;              // 環境を反映した視界距離

    float m_BaseFOV = 80.0f * DEG2RAD;    // 基準の視野角（ラジアン）
    float m_CurrentFOV = 60.0f * DEG2RAD;    // 環境を反映した視野角（ラジアン）
    bool m_HasLookedAtHeard = false;

    bool  m_IsFound = false;
};
