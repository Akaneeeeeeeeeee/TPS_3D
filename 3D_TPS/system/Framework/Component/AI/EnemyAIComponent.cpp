#include "EnemyAIComponent.h"
#include "Framework/Component/Physic/CharacterVirtualComponent.h"
#include "Framework/GameObject/GameObject.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>



void EnemyAIComponent::Attach(EngineContext& ctx)
{
    m_Physics = &ctx.joltPhysicsManager;
    m_Char = m_pOwner->GetComponent<CharacterVirtualComponent>();
}

void EnemyAIComponent::Detach(EngineContext& ctx)
{
    m_Physics = nullptr;
    m_Char = nullptr;
}

void EnemyAIComponent::Init(void)
{
	// キャラクターコンポーネントの取得
    if (!m_Char) { m_Char = m_pOwner->GetComponent<CharacterVirtualComponent>(); }

}

void EnemyAIComponent::Update(const float dt)
{
    if (!m_Char) { return; }
    if (m_WayPoints.empty())
    {
        m_Char->SetMoveDir(Vector3::Zero);
        return;
    }

    Vector3 pos = m_pOwner->GetPosition();

    // 1:ウェイポイントの更新
    Vector3 target = m_WayPoints[m_CurrentIndex];
    Vector3 toTarget = target - pos;
    if (toTarget.LengthSquared() < m_ArriveRadius * m_ArriveRadius)
    {
        m_CurrentIndex = (m_CurrentIndex + 1) % m_WayPoints.size();
        target = m_WayPoints[m_CurrentIndex];
        toTarget = target - pos;
    }

    if (toTarget.LengthSquared() < 0.0001f)
    {
        m_Char->SetMoveDir(Vector3::Zero);
        return;
    }

    Vector3 desired_dir = toTarget;
    desired_dir.Normalize();

    // 2:障害物回避
    Vector3 avoid_dir = ComputeAvoidDir(desired_dir);   // さっきの関数
    Vector3 move_dir = desired_dir + avoid_dir * m_AvoidWeight;

    if (move_dir.LengthSquared() > 0.0001f)
        move_dir.Normalize();
    else
        move_dir = Vector3::Zero;

    // 3:CharacterVirtual に入力
    m_Char->SetMoveDir(move_dir);
}

void EnemyAIComponent::Uninit(void)
{
    m_Char = nullptr;
    m_Physics = nullptr;
}

Vector3 EnemyAIComponent::ComputeAvoidDir(const Vector3& desired_dir)
{
    using namespace JPH;

    if (!m_Physics) { return Vector3::Zero; }

    auto& system = m_Physics->GetSystem();
    auto& npq = system.GetNarrowPhaseQuery();

    // キャラの位置と向き
    Vector3 fwd = desired_dir;      // とりあえず「進みたい方向」を前方とする

    if (fwd.LengthSquared() < std::numeric_limits<float>::epsilon()) { return Vector3::Zero; }

    fwd.Normalize();

    Vector3 pos = m_pOwner->GetPosition();
    float rayLen = m_RayLength;       // 例：200〜400

    // Ray の原点と方向
    RVec3 origin(pos.x, pos.y + m_EyeHeight, pos.z);   // 目の高さあたり
    Vec3  dir(fwd.x, fwd.y, fwd.z);

    // Ray は RRayCast を使う（RayCast ではない）
    RRayCast ray(origin, dir * rayLen);

    RayCastResult hit;

    // Character 自身や TRIGGER は無視するようにフィルタするのが理想
    auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);
    BodyFilter bodyFilter;        // 特に条件なければデフォルトで ok

    if (npq.CastRay(ray, hit, bpFilter, objFilter, bodyFilter))
    {
        // 0~1 の範囲（1 = rayLen の先端）
        float t = hit.mFraction;

        // 手前ほど強くよけたい
        float strength = 1.0f - t;

        // 左右どちらに避けるか、とりあえず左固定でもいいし、
        // 障害物の法線から決めてもいい
        Vec3 up = Vec3::sAxisY();
        Vec3 side = dir.Cross(up); // 左方向

        side = side.Normalized() * strength;

        return Vector3(side.GetX(), side.GetY(), side.GetZ());
    }

    return Vector3::Zero; // 障害物なし
}
