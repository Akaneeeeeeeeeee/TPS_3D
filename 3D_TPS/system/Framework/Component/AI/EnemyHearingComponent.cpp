#include "EnemyHearingComponent.h"
#include "Framework/GameObject/GameObject.h"
#include "EnemyAIComponent.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>

void EnemyHearingComponent::Attach(EngineServices& context)
{
	m_pPhysics = &context.physics;
	m_pWeather = &context.weather;
	m_pSound = &context.sound;
	m_pSound->RegisterListener(this);
}

void EnemyHearingComponent::Detach(void)
{
	if (m_pSound)
	{
		m_pSound->UnregisterListener(this); // 解除
		m_pSound = nullptr;
	}
	m_pPhysics = nullptr;
	m_pWeather = nullptr;
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

	// ---- 1) 天候＋時間による聴覚係数 ----
	float hearingFactor = 1.0f;
	if (m_pWeather)
	{
		hearingFactor = m_pWeather->GetHearingFactor();
	}

	// 「実効的な聴取半径」 = イベント半径 × 天候係数 × 自分の基準半径比
	float effectiveRadius = ev.Radius * hearingFactor;

	// さらに、敵ごとの個性を出したい場合：
	// m_BaseHearingRadius を「晴れの日の標準半径」とみなして、
	// ev.Radius がその標準に対してどれくらいの音かを見る形もあり。
	// ここではシンプルに ev.Radius をそのまま使うのでコメントアウト。
	// float enemyScale = m_BaseHearingRadius / 20.0f;  // 好みで基準を決める
	// effectiveRadius *= enemyScale;

	// ---- 2) 耳の位置と距離減衰 ----
	Vector3 earPos = m_pOwner->GetPosition();
	earPos.y += m_EarHeight;

	Vector3 toSound = ev.Position - earPos;
	float   dist = toSound.Length();
	if (dist <= 0.0001f) dist = 0.0001f;

	// 実効半径の外なら聞こえない
	if (dist > effectiveRadius)
		return 0.0f;

	float distFactor = 1.0f - (dist / effectiveRadius);
	distFactor = std::clamp(distFactor, 0.0f, 1.0f);

	// 遮蔽なしでの基準音量
	float baseLoudness = ev.Loudness * distFactor;

	// 天候でさらに減衰させる（豪雨などで全体の音量が落ちるイメージ）
	baseLoudness *= hearingFactor;

	// ここで「遮蔽なしで聞こえるか」を判定
	if (baseLoudness < m_Threshold)
		return 0.0f;

	// ---- 3) 遮蔽物による減衰 ----
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
}

void EnemyHearingComponent::SetEnemyAI(EnemyAIComponent* ai)
{
	m_pEnemyAI = ai;
}

void EnemyHearingComponent::OnWorldSound(const WorldSoundEvent& ev)
{
	if (!m_pEnemyAI) { return; }

	// 敵の足音には反応しない（敵同士の連鎖防止）
	if (ev.Emitter == SoundEmitterKind::Enemy && ev.Type == SoundType::Footstep)
		return;

	float perceived = ComputePerceivedLoudness(ev);

	// しきい値未満 → 無視
	//if (perceived < m_Threshold) { return; }
	// 0 以下なら「聞こえない」扱い
	if (perceived <= 0.0f) { return; }

	// AI に「この位置でこの強さの音がした」と知らせる
	m_pEnemyAI->OnHeardSound(ev.Position, perceived);
}
