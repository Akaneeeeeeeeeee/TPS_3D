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

/*
* @brief	音の聞こえ具合を計算する
* @param	ev		音イベント情報
* @return	聞こえ具合のスコア（0.0f 〜）
*/
float EnemyHearingComponent::ComputePerceivedLoudness(const WorldSoundEvent& ev) const
{
	if (!m_pOwner || !m_pPhysics)
		return 0.0f;

	// 耳の位置
	Vector3 earPos = m_pOwner->GetPosition();
	earPos.y += m_EarHeight;

	Vector3 toSound = ev.Position - earPos;
	float   dist = toSound.Length();
	if (dist <= 0.0001f) dist = 0.0001f;

	// 距離外ならそもそも聞こえない
	if (dist > ev.Radius)
		return 0.0f;

	float distFactor = 1.0f - (dist / ev.Radius);
	distFactor = std::clamp(distFactor, 0.0f, 1.0f);

	// 遮蔽物「なし」での基準音量
	float baseLoudness = ev.Loudness * distFactor;

	// ここで「遮蔽物なしで聞こえるか」を判定
	if (baseLoudness < m_Threshold)
		return 0.0f;

	// ここから遮蔽物による減衰
	using namespace JPH;

	auto& system = m_pPhysics->GetSystem();
	auto& npq = system.GetNarrowPhaseQuery();

	Vector3 dir3 = toSound / dist;
	RVec3 origin(earPos.x, earPos.y, earPos.z);
	Vec3  dir = ToJPH(dir3);

	RRayCast ray(origin, dir * dist);
	RayCastResult hit;

	// レイヤ設定
	auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::NON_MOVING);
	auto objFilter = system.GetDefaultLayerFilter(Layers::NON_MOVING);

	// 自分自身やトリガーを無視するフィルタ
	AvoidCharAndTriggerBodyFilter bodyFilter(system);

	bool blocked = npq.CastRay(
		ray,
		hit,
		bpFilter,
		objFilter,
		bodyFilter
	);

	float loudness = baseLoudness;

	if (blocked)
	{
		// hit.mFraction が 0〜1 で「どれくらい手前か」
		// 手前ほど強く減衰させてもいいが、まずは簡単に一定係数にする
		// ここで一枚の壁でどれくらい音が減るかを簡単に変えられる

		constexpr float OCCLUSION_FACTOR = 0.3f;
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
		//if (perceived < m_Threshold)
		if (perceived <= 0.0f)
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
	//if (perceived < m_Threshold) { return; }
	// 0 以下なら「聞こえない」扱い
	if (perceived <= 0.0f) { return; }

	// AI に「この位置でこの強さの音がした」と知らせる
	m_pEnemyAI->OnHeardSound(ev.Position, perceived);
}
