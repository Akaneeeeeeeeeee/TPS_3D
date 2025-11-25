#include "EnemyHearingComponent.h"
#include "Framework/GameObject/GameObject.h"
#include "EnemyAIComponent.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>

void EnemyHearingComponent::Attach(EngineContext& context)
{
    m_pPhysics = &context.joltPhysicsManager;
}

void EnemyHearingComponent::Detach(void)
{
    m_pPhysics = nullptr;
}

static JPH::Vec3 ToJPH(const Vector3& v)
{
    return JPH::Vec3(v.x, v.y, v.z);
}

float EnemyHearingComponent::ComputePerceivedLoudness(const WorldSoundEvent& ev) const
{
    // Owner or Physics or AI が無いなら聞こえない
    if (!m_pOwner || !m_pPhysics)
    {
        return 0.0f;
    }

    Vector3 myPos = m_pOwner->GetPosition();
    Vector3 toSound = ev.Position - myPos;

    float dist = toSound.Length();
    if (dist <= 0.0001f)
    {
        dist = 0.0001f;
    }

    // 1) 距離で減衰（半径を超えたら聞こえない）
    if (dist > ev.Radius)
    {
        return 0.0f;
    }

    // 距離 0 → 1.0, 距離 = Radius → 0.0
    float distFactor = 1.0f - (dist / ev.Radius);
    distFactor = std::clamp(distFactor, 0.0f, 1.0f);

    float loudness = ev.Loudness * distFactor;

    // 2) 遮蔽物チェック（Physics でレイキャスト）
    using namespace JPH;

    auto& system = m_pPhysics->GetSystem();
    auto& npq = system.GetNarrowPhaseQuery();

    // 耳の高さから音源へ
    JPH::RVec3 origin(
        myPos.x,
        myPos.y + m_EarHeight,
        myPos.z
    );

    Vector3 dir3 = ev.Position - myPos;
    dir3.Normalize();
    JPH::Vec3 dir = ToJPH(dir3);

    float maxDist = dist;

    JPH::RRayCast ray(origin, dir * maxDist);

    JPH::RayCastResult hit;

    // CHARACTER と同じフィルタ
    auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);
    JPH::BodyFilter  bodyFilter;

    bool blocked = npq.CastRay(
        ray,
        hit,
        bpFilter,
        objFilter,
        bodyFilter
    );

    if (blocked)
    {
        // hit.mFraction が 0〜1 で「どれくらい手前か」
        // 手前ほど強く減衰させてもいいが、まずは簡単に一定係数にする
        // ここで一枚の壁でどれくらい音が減るかを簡単に変えられる
        constexpr float OCCLUSION_FACTOR = 0.3f; // 壁1枚で 70% 減衰
        loudness *= OCCLUSION_FACTOR;
    }

    return loudness;
}


void EnemyHearingComponent::Update(const float dt)
{
    if (!m_pEnemyAI) { return; }

    const auto& events = SoundManager::GetInstance().GetEvents();

    for (const auto& ev : events)
    {
        float perceived = ComputePerceivedLoudness(ev);

        // 弱すぎる音は無視
        if (perceived < m_Threshold)
        {
            continue;
        }

        // 音を AI に通知
        m_pEnemyAI->OnHeardSound(ev.Position, perceived);
    }
}

void EnemyHearingComponent::SetEnemyAI(EnemyAIComponent* ai)
{
    m_pEnemyAI = ai;
}

void EnemyHearingComponent::OnWorldSound(const WorldSoundEvent& ev)
{
    if (!m_pEnemyAI) { return; }

    float perceived = ComputePerceivedLoudness(ev);

    // しきい値未満 → 無視
    if (perceived < m_Threshold) { return; }

    // AI に「この位置でこの強さの音がした」と知らせる
    m_pEnemyAI->OnHeardSound(ev.Position, perceived);
}
