#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"

// 前方宣言
class PhysicsManager;
class CharacterVirtualComponent;

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

    void UpdateIdle(const float deltatime);
    void UpdatePatrol(const float deltatime);
    void UpdateInvestigate(const float deltatime);
	void UpdateChase(const float deltatime);

    void Attach(EngineContext& ctx) override;
    void Detach(void) override;

    Vector3 ComputeAvoidDir(const Vector3& desired_dir);
    // 聴覚入力
    void OnHeardSound(const Vector3& pos)
    {
        m_LastHeardPosition = pos;
        m_State = Investigate;
    }

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

private:
    PhysicsManager* m_Physics = nullptr;
    CharacterVirtualComponent* m_Char = nullptr;

    State m_State = State::Patrol;
    Vector3 m_LastHeardPosition = Vector3::Zero;

    std::vector<Vector3> m_WayPoints;
    int   m_CurrentIndex = 0;
    float m_ArriveRadius = 50.0f;
    float m_RayLength = 300.0f;
    float m_AvoidWeight = 1.5f;
    float m_EyeHeight = 80.0f;
    // 調査状態用
    float m_InvestigateWaitTime = 2.0f; // 調査場所に到着後に何秒様子を見るか
    float m_InvestigateTimer = 0.0f;
};
