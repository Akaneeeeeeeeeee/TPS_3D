#include "EnemyAIComponent.h"
#include "Framework/Component/Physic/CharacterVirtualComponent.h"
#include "Framework/GameObject/GameObject.h"
#include "Framework/GameObject/Player/Player.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/LightSystem/LightSystem.h"
#include "Framework/Time/Time.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

namespace {
	// Y軸回転: 指定したベクトルをY軸（上方向）でradラジアン回転するユーティリティ関数
	static Vector3 RotateY(const Vector3& v, float rad)
	{
		float cosA = std::cos(rad);
		float sinA = std::sin(rad);
		return Vector3(
			v.x * cosA - v.z * sinA,
			v.y,
			v.x * sinA + v.z * cosA
		);
	}
}


void EnemyAIComponent::Attach(EngineServices& ctx)
{
	// 物理とキャラクター制御
	m_Physics = &ctx.physics;
	m_Char = m_pOwner->GetComponent<CharacterVirtualComponent>();

	// 天候・時間管理（視界パラメータの補正に使う）
	m_Weather = &ctx.weather;

	// ライトシステム（視界判定に使う）
	m_Light = &ctx.light;
}

void EnemyAIComponent::Detach(void)
{
	m_Physics = nullptr;
	m_Char = nullptr;
	m_Weather = nullptr;
}

void EnemyAIComponent::Init(void)
{
	// キャラクターコンポーネントの取得
	if (!m_Char)
	{
		m_Char = m_pOwner->GetComponent<CharacterVirtualComponent>();
	}

	// スタック監視初期化
	if (m_pOwner)
	{
		m_LastPosForStuck = m_pOwner->GetPosition();
	}
	m_StuckTimer = 0.0f;
	m_IsStuck = false;

	m_LastMoveDir = Vector3::Forward;
}

void EnemyAIComponent::Update(const float dt)
{
	if (!m_Char) { return; }

	// ------------------------------------------------------------
	// 1) 天候・時間に応じて「視界距離」「視野角」を更新する
	// ------------------------------------------------------------
	//float visibilityFactor = 1.0f;

	//// WeatherSystem 側で計算済みの「視認性係数」をもらう（0.0 ～ 1.0 想定）
	//float envVis = m_Weather ? m_Weather->GetVisibilityFactor() : 1.0f;

	//// envVis (0.1～1.0) を 0.7～1.0 に圧縮する
	//float f = 0.7f + 0.3f * envVis; // envVis=1 → f=1, envVis=0.1 → f=0.73
	//// 視界距離を係数でスケーリング
	//// 例: visibilityFactor = 0.5 → 視界距離 半分
	//m_CurrentViewDistance = m_BaseViewDistance * f;

	//// 視野角も暗いほど少し狭くする（好みで調整）
	//// visibilityFactor = 1.0 → fovScale = 1.0（昼は基準そのまま）
	//// visibilityFactor = 0.0 → fovScale = 0.7（真っ暗なら 70% 程度）
	//m_CurrentFOV = m_BaseFOV * f;

	m_CurrentViewDistance = m_BaseViewDistance;
	m_CurrentFOV = m_BaseFOV;


	// ------------------------------------------------------------
	// 2) 状態ごとの処理（移動や内部状態更新）
	// ------------------------------------------------------------
	switch (m_State)
	{
	case EnemyAIComponent::Idle:
		UpdateIdle(dt);
		break;
	case EnemyAIComponent::Caution:
		UpdateCaution(dt);   // ← ここで m_ViewForward を回す
		break;
	case EnemyAIComponent::Patrol:
		UpdatePatrol(dt);
		break;
	case EnemyAIComponent::Investigate:
		UpdateInvestigate(dt);
		break;
	default:
		break;
	}

	// ------------------------------------------------------------
	// 3) Caution 以外では「身体の向き」に視線を合わせる
	//    （警戒中だけは m_ViewForward を別管理）
	// ------------------------------------------------------------
	if (m_State != State::Caution && m_pOwner)
	{
		// モデルの前方（Z+）を視線として保持
		m_ViewForward = m_pOwner->GetForward();
		m_ViewForward.y = 0.0f;

		if (m_ViewForward.LengthSquared() > 1e-6f)
		{
			m_ViewForward.Normalize();
		}
		else
		{
			m_ViewForward = Vector3::Forward;
		}
	}

	// ------------------------------------------------------------
	// 4) 最終的な視界情報を使ってプレイヤーの視認判定を行う
	// ------------------------------------------------------------
	UpdateSight(dt);
}

void EnemyAIComponent::Uninit(void)
{
	m_Char = nullptr;
	m_Physics = nullptr;
}

/*
* @brief	進行方向に障害物がある場合に回避方向を計算する
* @detail	RayCastを使って障害物を検出し、回避方向ベクトルを返す
* @param	desired_dir	進みたい方向ベクトル（正規化されているもの）
* @return	回避方向ベクトル（正規化されていない）
*/
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

void EnemyAIComponent::UpdateIdle(const float deltatime)
{
}

void EnemyAIComponent::UpdatePatrol(const float dt)
{
	if (m_WayPoints.empty())
	{
		m_Char->SetMoveDir(Vector3::Zero);
		m_State = Idle;
		return;
	}

	Vector3 pos = m_pOwner->GetPosition();

	// ----------------------------
	// (A) 振り向き中フェーズ
	// ----------------------------
	if (m_IsPatrolTurning)
	{
		m_Char->SetMoveInput(Vector3::Zero, 0.0f);

		m_PatrolTurnTime += dt;
		float t = (m_PatrolTurnDuration > 1e-6f) ? (m_PatrolTurnTime / m_PatrolTurnDuration) : 1.0f;
		if (t > 1.0f) t = 1.0f;

		// 最短角で補間するため、m_PatrolTargetYaw は「最短にした値」を入れておく前提
		float yaw = std::lerp(m_PatrolStartYaw, m_PatrolTargetYaw, t);
		m_pOwner->SetRotation(Quaternion::CreateFromAxisAngle(Vector3::Up, yaw));

		if (t >= 1.0f)
		{
			m_IsPatrolTurning = false;
			m_CurrentIndex = m_PatrolNextIndex;
		}
		return;
	}

	// 現在目標
	Vector3 target = m_WayPoints[m_CurrentIndex];
	Vector3 toTarget = target - pos;
	float distSq = toTarget.LengthSquared();

	// ----------------------------
	// (B) 巡回点に到着 → 次の方向へ振り向き開始
	// ----------------------------
	// 目標地点に十分近づいたら次のウェイポイントへ
	if (distSq < m_ArriveRadius * m_ArriveRadius)
	{
		const int nextIndex = (m_CurrentIndex + 1) % (int)m_WayPoints.size();

		Vector3 toNext = m_WayPoints[nextIndex] - pos;
		toNext.y = 0.0f;

		if (toNext.LengthSquared() > 1e-6f)
		{
			toNext.Normalize();

			Vector3 forward = m_pOwner->GetForward();
			forward.y = 0.0f;
			if (forward.LengthSquared() < 1e-6f) forward = Vector3::Forward;
			forward.Normalize();

			// 右/左（アニメ選択用）
			float crossY = forward.x * toNext.z - forward.z * toNext.x;
			m_PatrolTurnRight = (crossY > 0.0f);

			// ヨー角（今と目標）
			float startYaw = std::atan2(-forward.x, -forward.z);
			float targetYaw = std::atan2(-toNext.x, -toNext.z);

			// 最短角へ正規化
			float delta = targetYaw - startYaw;
			while (delta > PI)  delta -= 2.0f * PI;
			while (delta < -PI) delta += 2.0f * PI;

			m_PatrolStartYaw = startYaw;
			m_PatrolTargetYaw = startYaw + delta;

			m_PatrolTurnTime = 0.0f;
			m_PatrolNextIndex = nextIndex;
			m_IsPatrolTurning = true;

			// このフレームは止まって振り向き開始
			m_Char->SetMoveInput(Vector3::Zero, 0.0f);
			return;
		}

		// 次が同一点っぽいなら普通に進める
		m_CurrentIndex = nextIndex;
		target = m_WayPoints[m_CurrentIndex];
		toTarget = target - pos;
		distSq = toTarget.LengthSquared();
	}

	// ----------------------------
	// (C) 通常の巡回移動
	// ----------------------------
	// 障害物回避込みの移動方向
	Vector3 moveDir = ComputeMoveDirToTarget(target);

	// スタック検出：どこへ行きたいか（toTarget）を渡す
	UpdateStuck(dt, toTarget);

	if (moveDir.LengthSquared() > 0.0001f)
	{
		// 正規化してセット
		moveDir.Normalize();
		m_Char->SetMoveInput(moveDir, m_PatrolMoveAmount);
		// キャラの向きも合わせる
		FaceMoveDir(moveDir);
	}
	else
	{
		m_Char->SetMoveInput(Vector3::Zero, 0.0f);
	}
}

void EnemyAIComponent::UpdateInvestigate(const float dt)
{
	if (!m_HasInvestigateTarget)
	{
		// 目的地が無いなら巡回へ戻すなど
		m_State = m_WayPoints.empty() ? Idle : Patrol;
		return;
	}

	Vector3 pos = m_pOwner->GetPosition();

	// 未正規化：目標までの「距離」と「方向」を持ったまま
	Vector3 desired = m_InvestigateTarget - pos;    // 調査対象の位置
	float distSq = desired.LengthSquared();

	// 1) まだ到達してない → 移動
	if (distSq > m_ArriveRadius * m_ArriveRadius)
	{
		m_InvestigateTimer = 0.0f;

		Vector3 moveDir = ComputeMoveDirToTarget(m_InvestigateTarget);

		// 調査中でもスタック監視・解決
		UpdateStuck(dt, desired);

		if (moveDir.LengthSquared() > 0.0001f)
		{
			moveDir.Normalize();
			m_Char->SetMoveInput(moveDir, m_InvestigateMoveAmount);
			FaceMoveDir(moveDir);
		}
		else
		{
			m_Char->SetMoveInput(Vector3::Zero, 0.0f);
		}
		return;
	}

	// 2) 到達した → 待機
	m_Char->SetMoveInput(Vector3::Zero, 0.0f);

	// 到達中はスタック扱いしない（距離履歴をリセット）
	m_StuckTimer = 0.0f;
	m_IsStuck = false;
	m_HasLastDistToTarget = false;

	m_InvestigateTimer += dt;
	if (m_InvestigateTimer >= m_InvestigateWaitTime)
	{
		m_InvestigateTimer = 0.0f;
		m_State = m_WayPoints.empty() ? Idle : Patrol;
	}
}

/*
* @brief	スタック状態の解消処理
*/
void EnemyAIComponent::UpdateStuck(float dt, const Vector3& desiredDir)
{
	if (!m_pOwner || !m_Char) return;

	// ほぼ「移動する意図」が無いときは、スタック判定しない
	if (desiredDir.LengthSquared() < 1.0f)
	{
		m_StuckTimer = 0.0f;
		m_IsStuck = false;
		m_LastPosForStuck = m_pOwner->GetPosition();
		m_HasLastDistToTarget = false; // 距離履歴もリセット
		return;
	}

	Vector3 nowPos = m_pOwner->GetPosition();
	float movedSq = (nowPos - m_LastPosForStuck).LengthSquared();

	// 今フレームの「目標までの距離」
	float distToTarget = desiredDir.Length();

	// 初回は履歴がないので、ここで保存して終わる
	if (!m_HasLastDistToTarget)
	{
		m_HasLastDistToTarget = true;
		m_LastDistToTarget = distToTarget;
		m_StuckTimer = 0.0f;
		m_LastPosForStuck = nowPos;
		return;
	}

	// しきい値
	const float MOVE_EPS_SQ = 1.0f; // ほとんど動いていない距離
	const float TARGET_EPS = 5.0f;  // 目標までの距離が「縮んだ」とみなす量
	const float STUCK_TIME = 1.0f;  // 何秒続いたらスタックとみなすか

	// 位置としては動いているか
	bool movingPos = (movedSq > MOVE_EPS_SQ);

	// 目標への距離がちゃんと縮んでいるか
	// 「前フレームより TARGET_EPS 以上近くなっている」なら progress とみなす
	bool closingTarget = (distToTarget < m_LastDistToTarget - TARGET_EPS);

	// 位置がほとんど動いていない か 目標への距離がほとんど縮んでいない場合
	if (!movingPos || !closingTarget)
	{
		m_StuckTimer += dt;
	}
	else
	{
		// ちゃんと前進できているのでリセット
		m_StuckTimer = 0.0f;
		m_IsStuck = false;
	}

	// 状態更新
	m_LastPosForStuck = nowPos;
	m_LastDistToTarget = distToTarget;

	if (m_StuckTimer > STUCK_TIME)
	{
		m_IsStuck = true;
		ResolveStuck();
		m_StuckTimer = 0.0f;

		// 次回のために距離履歴をリセットしておく
		m_HasLastDistToTarget = false;
	}
}

void EnemyAIComponent::FaceMoveDir(const Vector3& moveDir)
{
	if (moveDir.LengthSquared() <= 0.0001f) { return; }

	Vector3 dir = moveDir;
	dir.y = 0.0f;
	dir.Normalize();

	// モデルの回転だけはz-前方系のヨー角に合わせる
	float yaw = std::atan2(-dir.x, -dir.z);

	Quaternion q = Quaternion::CreateFromAxisAngle(Vector3::Up, yaw);
	m_pOwner->SetRotation(q);
}

void EnemyAIComponent::OnHeardSound(const Vector3& pos, float strength)
{
	if (!m_pOwner) { return; }

	// 常に「最後に聞いた音」を更新（怪しみ中でも更新する）
	m_LastHeardPosition = pos;
	m_HeardThisFrame = true; // Enemy側の驚きアニメ用トリガ

	// 調査地点は常に音源へ
	m_InvestigateTarget = pos;
	m_HasInvestigateTarget = true;

	// 角度チェックは必要なら残す（正面ほぼ一致なら直接 Investigate へ）
	Vector3 selfPos = m_pOwner->GetPosition();
	Vector3 toSound = pos - selfPos;
	toSound.y = 0.0f;

	// ほぼ同一点なら回転は要らないのでそのまま Investigate へ
	if (toSound.LengthSquared() < 1e-4f)
	{
		m_State = Investigate;
		m_InvestigateTimer = 0.0f;
		m_WallFollowSide = 0;
		m_IsAvoidingWall = false;
		return;
	}

	toSound.Normalize();

	// 怪しんでるときに新しい音が来たら、そちらを優先して振り向き直す
	if (m_State == State::Caution)
	{
		// いまの視線方向から、新しい音方向へ“回転やり直し”
		m_CautionTurnTime = 0.0f;
		m_CautionWaitTime = 0.0f;
		m_HasLookedAtHeard = false;

		m_CautionStartViewDir = GetViewForward(); // 現在視線（Caution中はm_ViewForward）
		m_CautionTargetViewDir = toSound;

		// 状態はCautionのまま維持（音を優先して見直す）
		return;
	}

	// 現在向いている方向（Transform は Z- 前方系）
	Quaternion rot = m_pOwner->GetRotation();
	Vector3 forward = Vector3::Transform(Vector3::Forward, rot);
	forward.y = 0.0f;

	if (forward.LengthSquared() < 1e-4f)
	{
		forward = Vector3::Forward;
	}

	forward.Normalize();

	float currentYaw = std::atan2(-forward.x, -forward.z);
	float targetYaw = std::atan2(-toSound.x, -toSound.z);

	float delta = targetYaw - currentYaw;
	while (delta > PI) delta -= 2.0f * PI;
	while (delta < -PI) delta += 2.0f * PI;

	if (std::fabs(delta) < 0.1f)
	{
		// ほぼ正面なら驚き挟まずに即 Investigate
		m_State = Investigate;
		m_InvestigateTimer = 0.0f;
		m_WallFollowSide = 0;
		m_IsAvoidingWall = false;
		return;
	}

	// Caution 状態へ
	m_CautionTurnTime = 0.0f;
	m_CautionWaitTime = 0.0f;
	m_HasLookedAtHeard = false; // まだ音源方向を見ていない
	// 視線の開始方向・目標方向をセット
	m_CautionStartViewDir = forward;   // 今の視線
	m_CautionTargetViewDir = toSound;   // 音の方向
	m_State = Caution;
}

Vector3 EnemyAIComponent::GetEyePosition(void) const
{
	if (!m_pOwner) return Vector3::Zero;

	Vector3 pos = m_pOwner->GetPosition();
	pos.y += m_EyeHeight;
	return pos;
}

void EnemyAIComponent::UpdateSight(const float dt)
{
	if (!m_pPlayer || !m_Physics || !m_pOwner) { return; }
	UpdateSuspicionFromSight(dt);

	// 一点でも“見えてる扱い”なら警戒へ（音の警戒と混ぜたくないなら分岐を作る）
	if (m_CanSeeAnyPointThisFrame)
	{
		if (m_State == State::Idle || m_State == State::Patrol || m_State == State::Investigate)
		{
			// 既存の Caution を使うなら、視線ターゲットだけプレイヤー方向に差し替える
			Vector3 eyePos = GetEyePosition();
			Vector3 targetPos = m_HasLastSeenPos ? m_LastSeenPos : m_pPlayer->GetPosition();
			Vector3 to = targetPos - eyePos;
			//to.y = 0.0f;
			if (to.LengthSquared() > 1e-6f)
			{
				to.Normalize();
				m_CautionTurnTime = 0.0f;
				m_CautionWaitTime = 0.0f;
				m_CautionStartViewDir = GetViewForward();
				m_CautionTargetViewDir = to;
				m_HasLookedAtHeard = false;
				m_State = State::Caution;
			}
		}
	}

	// 不審度MAXで発見
	if (m_Suspicion >= 1.0f)
	{
		m_Suspicion = 1.0f;
		m_IsFound = true;
	}
}

/*
* @brief	視線情報から不審度を更新する
*/
void EnemyAIComponent::UpdateSuspicionFromSight(float dt)
{
	Vector3 eyePos = GetEyePosition();

	std::vector<Vector3> samples;
	m_pPlayer->GetVisibilitySamplePoints(eyePos, samples);
	int samplePoint = std::min<int>((int)samples.size(), SamplePointCount);

	m_CanSeeAnyPointThisFrame = false;

	int   visibleNowCount = 0;
	float sumContribution = 0.0f;

	// 環境の基本見えやすさ（昼=1, 夜=小さい）
	float env01 = m_Weather ? m_Weather->GetVisibilityFactor() : 1.0f;
	env01 = std::clamp(env01, 0.0f, 1.0f);

	// 「暗い＆遠い」は“見えてる扱いにしない”ためのしきい値
	constexpr float VISIBLE_MIN = 0.15f; // 調整用。通常は空namespaceに。

	float bestC = -1.0f;
	Vector3 bestP = Vector3::Zero;

	// 各サンプル点について視認判定
	for (int i = 0; i < samplePoint; ++i)
	{
		const Vector3& p = samples[i];

		bool inCone = IsInViewCone(eyePos, p);
		bool los = inCone ? CanSeePoint(eyePos, p) : false;

		float dist = (p - eyePos).Length();
		// 距離の減衰（0..1）
		float distMul = 1.0f - std::clamp(dist / std::max(1.0f, m_BaseViewDistance), 0.0f, 1.0f);

		// ライトによる見えやすさ補正（0..1）
		float light01 = (m_Light) ? std::clamp(m_Light->GetLightVisibility01(p), 0.0f, 1.0f) : 0.0f;

		// 夜の暗さをライトが押し上げる
		float bright01 = env01 + light01 * (1.0f - env01); // 0..1

		// 暗すぎる＆遠すぎる点は “見えてる扱い” にしない
		bool visibleNow = (inCone && los && (distMul * bright01 >= VISIBLE_MIN));

		if (visibleNow)
		{
			m_SeenSec[i] += dt;
			++visibleNowCount;
			m_CanSeeAnyPointThisFrame = true;
		}
		else
		{
			// 見えてないなら見えてた時間を減衰
			m_SeenSec[i] = std::max(0.0f, m_SeenSec[i] - dt * m_SeenDecayPerSec);
		}

		if (!visibleNow)
			continue; // 見えてる扱いの点だけ寄与

		// 見えてる点の寄与度計算
		float hold = HoldTimeByDistance(dist);
		float conf = (hold > 1e-6f) ? (m_SeenSec[i] / hold) : 1.0f;
		conf = std::clamp(conf, 0.0f, 1.0f);

		// 点の寄与度合計に加算
		float contribution = conf * distMul * bright01;
		sumContribution += contribution;

		// 一番寄与が大きい点を「最後に見えた点」にする候補
		if (contribution > bestC)
		{
			bestC = contribution;
			bestP = p;
		}
	}

	// 見えてるなら last seen を更新し、調査目標もそれにする
	if (m_CanSeeAnyPointThisFrame)
	{
		m_LastSeenPos = bestP;
		m_HasLastSeenPos = true;

		m_InvestigateTarget = m_LastSeenPos;
		m_HasInvestigateTarget = true;
	}

	// 4点平均（n で割る）
	float sight01 = (samplePoint > 0) ? (sumContribution / static_cast<float>(samplePoint)) : 0.0f;

	float countMul = m_PointCountMul[std::clamp(visibleNowCount, 0, SamplePointCount)];

	if (sight01 > 0.0f)
		m_Suspicion += dt * m_SusGainPerSec * sight01 * countMul;
	else
		m_Suspicion -= dt * m_SusLosePerSec;

	m_Suspicion = std::clamp(m_Suspicion, 0.0f, 1.0f);
}


/*
* @brief	プレイヤーが見えているかを判定する
*/
bool EnemyAIComponent::CanSeePlayer(void) const
{
	if (!m_pPlayer || !m_Physics || !m_pOwner) { return false; }

	Vector3 eyePos = GetEyePosition();

	std::vector<Vector3> samples;
	m_pPlayer->GetVisibilitySamplePoints(eyePos, samples);

	for (const auto& p : samples)
	{
		// まず視野角・視距離の円錐内か？
		if (!IsInViewCone(eyePos, p))
			continue;

		// 敵の目から、プレイヤーのカプセル上の点までの間に、壁などの“遮蔽物”があるか？
		if (CanSeePoint(eyePos, p))
			return true;
	}

	return false;
}


/*
* @brief	ターゲット位置が視野円錐内にあるかを判定する
*/
bool EnemyAIComponent::IsInViewCone(
	const Vector3& eyePos,
	const Vector3& targetPos) const
{
	// 目からターゲットへのベクトル
	Vector3 toTarget = targetPos - eyePos;
	float   dist = toTarget.Length();

	// 距離が 0 に近い or 視界距離を超えているなら見えない
	if (dist <= 0.0001f || dist > m_CurrentViewDistance)
		return false;

	// 方向ベクトルに正規化
	Vector3 dir = toTarget / dist;

	// 現在の視線方向（Caution 中は m_ViewForward、それ以外は Update で身体向きから設定済み）
	Vector3 forward = GetViewForward();
	if (forward.LengthSquared() < 1e-6f)
	{
		forward = Vector3::Forward;
	}
	forward.Normalize();

	// forward と dir のなす角の cos 値を計算
	float cosAngle = forward.Dot(dir);

	// 現在の視野角（ラジアン）の半分を使って cos(θ/2) を求める
	float cosHalfFov = std::cos(m_CurrentFOV * 0.5f);

	// cos(angle) が cos(fov/2) 以上なら視野円錐の中にある
	return cosAngle >= cosHalfFov;
}

/*
* @brief	ターゲット位置が見えているかを判定する
*/
bool EnemyAIComponent::CanSeePoint(const Vector3& eyePos, const Vector3& targetPos) const
{
	using namespace JPH;
	if (!m_Physics) { return false; }

	Vector3 toTarget = targetPos - eyePos;
	float   dist = toTarget.Length();
	if (dist <= 0.0001f)
		return false;

	Vector3 dir = toTarget / dist;

	auto& system = m_Physics->GetSystem();
	auto& npq = system.GetNarrowPhaseQuery();

	RVec3 origin(eyePos.x, eyePos.y, eyePos.z);
	Vec3  jdir(dir.x, dir.y, dir.z);

	RRayCast ray(origin, jdir * dist);
	RayCastResult hit;

	// 「キャラから見て何に当たるか」という組み合わせを使う
	auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
	auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);

	// 遮蔽物だけを対象にしたいので「キャラとトリガーは無視」
	AvoidCharAndTriggerBodyFilter bodyFilter(system);

	// hit == true  → 遮蔽物あり
	// hit == false → 遮蔽物なし
	bool blocked = npq.CastRay(ray, hit, bpFilter, objFilter, bodyFilter);

	// 遮蔽物がなければ「その点は見えている」
	return !blocked;
}


/*
* @brief	ターゲット位置に向かうための移動方向を計算する
* @detail	障害物を避けつつターゲットに向かう方向ベクトルを返す
* @param	target		目標位置
* @return	移動方向ベクトル（正規化されている）
*/
Vector3 EnemyAIComponent::ComputeMoveDirToTarget(const Vector3& target)
{
	using namespace JPH;

	if (!m_Physics) { return Vector3::Zero; }

	auto& system = m_Physics->GetSystem();
	auto& npq = system.GetNarrowPhaseQuery();

	Vector3 pos = m_pOwner->GetPosition();

	Vector3 toTarget = target - pos;
	toTarget.y = 0.0f;

	float distToTarget = toTarget.Length();
	if (distToTarget < m_ArriveRadius * 0.5f)
	{
		m_IsAvoidingWall = false;
		m_AvoidMode = AvoidMode::None;
		m_WallFollowSide = 0;
		m_ClearTimer = 0.0f;
		return Vector3::Zero;
	}

	Vector3 forward = toTarget / distToTarget;

	Vector3 origin3 = pos;
	origin3.y += m_EyeHeight;

	float rayLen = m_RayLength;
	float maxCheckDist = std::min(rayLen, distToTarget);

	float blockThreshold = maxCheckDist * 0.8f;

	auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
	auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);
	AvoidCharAndTriggerBodyFilter bodyFilter(system);

	auto castDist = [&](const Vector3& dir3) -> float
		{
			Vector3 d = dir3;
			d.y = 0.0f;
			if (d.LengthSquared() < 0.0001f) { return maxCheckDist; }
			d.Normalize();

			RVec3 origin(origin3.x, origin3.y, origin3.z);
			Vec3  jdir(d.x, d.y, d.z);

			RRayCast      ray(origin, jdir * maxCheckDist);
			RayCastResult hit;

			if (npq.CastRay(ray, hit, bpFilter, objFilter, bodyFilter))
				return hit.mFraction * maxCheckDist;

			return maxCheckDist;
		};

	// 正面の空き
	float centerFree = castDist(forward);
	bool frontBlocked = (centerFree < blockThreshold);

	// ----------------------------
	// 解除判定（壁沿い中の揺れ防止）
	// ----------------------------
	constexpr float CLEAR_SEC = 0.15f; // 少しだけ“空きが続いたら”解除
	if (!frontBlocked)
	{
		if (m_AvoidMode == AvoidMode::WallFollow)
		{
			//m_ClearTimer += Time::GetInstance().Deltatime(); // dt を渡してるなら dt を使ってOK
			//if (m_ClearTimer < CLEAR_SEC)
			//{
			//	// まだ解除しない（ちょっとだけ壁沿い継続）
			//	frontBlocked = true;
			//}
			if (!frontBlocked)
			{
				m_ClearTimer += Time::GetInstance().Deltatime();
				if (m_ClearTimer < CLEAR_SEC)
				{
					frontBlocked = true; // まだ壁沿いを続ける
				}
			}
			else
			{
				m_ClearTimer = 0.0f; // 連続で空いた時だけ解除
			}
		}
	}

	if (!frontBlocked)
	{
		m_IsAvoidingWall = false;
		m_AvoidMode = AvoidMode::None;
		m_WallFollowSide = 0;
		m_ClearTimer = 0.0f;

		m_LastMoveDir = forward;
		return forward;
	}

	// ここから回避モード
	bool just_started_avoid = false;
	if (!m_IsAvoidingWall)
	{
		m_IsAvoidingWall = true;
		just_started_avoid = true;

		// 回避開始で軽く初期化
		m_AvoidMode = AvoidMode::Steer;
		m_ClearTimer = 0.0f;
		m_WallFollowSide = 0;
	}

	// 近いほど強い
	float nearFactor = 1.0f - (centerFree / blockThreshold);
	nearFactor = std::clamp(nearFactor, 0.0f, 1.0f);

	float steer = std::clamp(nearFactor * m_AvoidWeight, 0.0f, 1.0f);

	constexpr float DEG2RAD_LOCAL = PI / 180.0f;

	// ------------------------------------------------------------
	// A) 壁沿い中は“角度探索に戻らない” ＝安定化
	// ------------------------------------------------------------
	if (m_AvoidMode != AvoidMode::WallFollow)
	{
		// ------------------------------------------------------------
		// 1) 角度探索：小さく曲げて抜けられるならそれを使う
		// ------------------------------------------------------------
		const float anglesDeg[] = { 10, -10, 20, -20, 30, -30, 45, -45, 60, -60, 90, -90 };

		Vector3 bestDir = forward;
		float bestFree = centerFree;
		bool foundGood = false;

		for (float deg : anglesDeg)
		{
			Vector3 cand = RotateY(forward, deg * DEG2RAD_LOCAL);
			float freeD = castDist(cand);

			if (freeD >= blockThreshold)
			{
				bestDir = cand;
				foundGood = true;
				break;
			}

			if (freeD > bestFree)
			{
				bestFree = freeD;
				bestDir = cand;
			}
		}

		if (foundGood)
		{
			m_AvoidMode = AvoidMode::Steer;

			Vector3 moveDir = LerpDir(forward, bestDir, steer);
			moveDir.y = 0.0f;
			if (moveDir.LengthSquared() < 0.0001f) moveDir = bestDir;
			moveDir.Normalize();

			if (just_started_avoid)
			{
				m_LastMoveDir = moveDir;
				return moveDir;
			}

			m_LastMoveDir = LerpDir(m_LastMoveDir, moveDir, 0.2f);
			return m_LastMoveDir;
		}

		// ------------------------------------------------------------
		// 2) 抜けられない → 壁沿いへ移行
		// ------------------------------------------------------------
		m_AvoidMode = AvoidMode::WallFollow;
		m_ClearTimer = 0.0f;

		// 左右どちらに沿うか決める（±90度）
		Vector3 leftT = RotateY(forward, +90.0f * DEG2RAD_LOCAL);
		Vector3 rightT = RotateY(forward, -90.0f * DEG2RAD_LOCAL);
		float leftFree = castDist(leftT);
		float rightFree = castDist(rightT);

		m_WallFollowSide = (leftFree >= rightFree) ? +1 : -1;
	}

	// ------------------------------------------------------------
	// B) 壁沿い：forward ではなく tangent（±90）を強めに使う
	// ------------------------------------------------------------
	Vector3 tangent = RotateY(forward, (float)m_WallFollowSide * 90.0f * DEG2RAD_LOCAL);
	tangent.y = 0.0f;
	if (tangent.LengthSquared() > 1e-6f) tangent.Normalize();
	else tangent = forward;

	// 壁沿いは弱いと“壁に刺さる”ので最低値を持たせる
	float wallSteer = std::max(steer, 0.65f);

	Vector3 moveDir = LerpDir(forward, tangent, wallSteer);
	moveDir.y = 0.0f;
	if (moveDir.LengthSquared() < 0.0001f) moveDir = tangent;
	moveDir.Normalize();

	if (just_started_avoid)
	{
		m_LastMoveDir = moveDir;
		return moveDir;
	}

	m_LastMoveDir = LerpDir(m_LastMoveDir, moveDir, 0.2f);
	return m_LastMoveDir;
}


bool EnemyAIComponent::ConsumeHeardSoundPosition(Vector3& outPos)
{
	if (!m_HeardThisFrame) { return false; }

	outPos = m_LastHeardPosition;
	m_HeardThisFrame = false; // 一度読んだらクリア
	return true;
}

void EnemyAIComponent::UpdateCaution(const float dt)
{
	// その場で停止
	if (m_Char) {
		m_Char->SetMoveDir(Vector3::Zero);
	}

	// 1:視線回転フェーズ
	if (m_CautionTurnTime < m_CautionTurnDuration)
	{
		m_CautionTurnTime += dt;
		float t = m_CautionTurnTime / m_CautionTurnDuration;
		if (t > 1.0f)
		{
			t = 1.0f;
		}
		// 視線だけ補間
		m_ViewForward = LerpDir(m_CautionStartViewDir, m_CautionTargetViewDir, t);

		return; // 回転中は待機フェーズに入らない
	}

	// 完全にターゲット方向を向いた状態
	m_ViewForward = m_CautionTargetViewDir;
	// 回転完了した瞬間に一度だけ本体の向きを音方向にスナップ
	if (!m_HasLookedAtHeard && m_pOwner)
	{
		m_HasLookedAtHeard = true;

		Vector3 dir = m_CautionTargetViewDir;   // Z+ 前方系
		float yaw = std::atan2(-dir.x, -dir.z); // モデルが Z- 前方なのでこの計算

		Quaternion q = Quaternion::CreateFromYawPitchRoll(yaw, 0.0f, 0.0f);
		m_pOwner->SetRotation(q);
	}

	// 2:待機フェーズ
	m_CautionWaitTime += dt;
	if (m_CautionWaitTime >= m_CautionWaitDuration)
	{
		// 調査状態へ移行
		m_State = Investigate;
		m_InvestigateTimer = 0.0f;
		m_WallFollowSide = 0;
		m_AvoidMode = AvoidMode::None;
		m_ClearTimer = 0.0f;
		m_IsAvoidingWall = false;
	}
}

// スタック状態の解消処理
void EnemyAIComponent::ResolveStuck(void)
{
	if (!m_pOwner || !m_Char) return;

	// キャラの半径を基準に最大探索半径を決める
	float charRadius = m_Char ? m_Char->GetRadius() : 50.0f;

	// ベース半径: キャラ半径の 20 倍
	const float BASE_MAX_R = charRadius * 20.0f;
	// 上限半径: キャラ半径の 50 倍
	const float MAX_MAX_R = charRadius * 50.0f;
	// 1 回失敗するごとに少しだけ広げる倍率
	const float GROW_FACTOR = 1.3f;

	float dynamicMaxR =
		BASE_MAX_R * std::pow(GROW_FACTOR,
			static_cast<float>(m_StuckResolveCount));
	if (dynamicMaxR > MAX_MAX_R)
		dynamicMaxR = MAX_MAX_R;

	Vector3 escapePos;
	if (FindLocalEscape(escapePos, dynamicMaxR))
	{
		// 見つかったらテレポート
		m_Char->Teleport(escapePos);

		m_IsAvoidingWall = false;
		m_StuckTimer = 0.0f;
		m_LastPosForStuck = escapePos;
		m_StuckResolveCount = 0;      // 成功したのでリセット
		m_HasLastDistToTarget = false;
		return;
	}

	// 見つからなかった場合は次回探索時に半径を広げる
	++m_StuckResolveCount;

	// 必要ならここでウェイポイントワープのフォールバックを呼ぶ
	// ResolveStuckFallback();
}


// ローカル範囲での脱出地点探索
//bool EnemyAIComponent::FindLocalEscape(Vector3& outPos, const float maxR)
//{
//	if (!m_Physics || !m_pOwner) { return false; }
//	if (!m_TerrainCol) { return false; }
//
//	Vector3 center = m_pOwner->GetPosition();
//	Vector3 forward = m_pOwner->GetForward();
//	forward.y = 0.0f;
//	if (forward.LengthSquared() < 1e-4f)
//	{
//		forward = Vector3::Forward;
//	}
//	forward.Normalize();
//
//	// チェックする角度の一覧（度）
//	static const float anglesDeg[] =
//	{ 0.0f, 45.0f, -45.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f };
//
//	// キャラの直径を使う
//	float radius = m_Char ? m_Char->GetRadius() : 50.0f;
//	float charDiameter = radius * 2.0f;
//
//	// 半径の刻み
//	const float stepR = charDiameter; // 「キャラ1人分ずつ」外側へ
//	const float footOffset = 2.0f;
//
//	for (float r = stepR; r <= maxR; r += stepR)
//	{
//		for (float deg : anglesDeg)
//		{
//			float rad = deg * DEG2RAD;
//			Vector3 dir = RotateY(forward, rad);
//			dir.y = 0.0f;
//			if (dir.LengthSquared() < 1e-6f) continue;
//			dir.Normalize();
//
//			Vector3 xz = center + dir * r;
//
//			float groundY;
//			if (!m_TerrainCol->SampleHeight(xz.x, xz.z, groundY))
//				continue;
//
//			Vector3 candidate(xz.x, groundY + footOffset, xz.z);
//
//			if (IsCapsuleFree(candidate))
//			{
//				outPos = candidate;
//				return true;
//			}
//		}
//	}
//	return false;
//}
bool EnemyAIComponent::FindLocalEscape(Vector3& outPos, const float maxR)
{
	if (!m_Physics || !m_pOwner) { return false; }
	if (!m_TerrainCol) { return false; }

	Vector3 center = m_pOwner->GetPosition();

	// 現在の足元Y（ownerのPositionが足元基準の前提。もし腰基準ならここを補正）
	const float curY = center.y;

	Vector3 forward = m_pOwner->GetForward();
	forward.y = 0.0f;
	if (forward.LengthSquared() < 1e-4f)
		forward = Vector3::Forward;
	forward.Normalize();

	static const float anglesDeg[] =
	{ 0.0f, 45.0f, -45.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f };

	float radius = m_Char ? m_Char->GetRadius() : 50.0f;
	float charDiameter = radius * 2.0f;

	const float stepR = charDiameter;
	const float footOffset = 2.0f;

	// 高低差許容（調整値）
	// 例：段差の上下がこれを超える候補は除外
	constexpr float MAX_Y_DELTA = 200.0f;

	for (float r = stepR; r <= maxR; r += stepR)
	{
		for (float deg : anglesDeg)
		{
			float rad = deg * DEG2RAD;
			Vector3 dir = RotateY(forward, rad);
			dir.y = 0.0f;
			if (dir.LengthSquared() < 1e-6f) continue;
			dir.Normalize();

			Vector3 xz = center + dir * r;

			float groundY;
			if (!m_TerrainCol->SampleHeight(xz.x, xz.z, groundY))
				continue;

			Vector3 candidate(xz.x, groundY + footOffset, xz.z);

			// 高低差フィルタ
			// 現在の高さから離れすぎる候補は「別の段差/崖下」へのワープになりやすいので捨てる
			if (std::fabs(candidate.y - curY) > MAX_Y_DELTA)
				continue;

			if (IsCapsuleFree(candidate))
			{
				outPos = candidate;
				return true;
			}
		}
	}
	return false;
}


float EnemyAIComponent::HoldTimeByDistance(float dist) const
{
	float view = std::max(1.0f, m_CurrentViewDistance);
	float t = std::clamp(dist / view, 0.0f, 1.0f);
	return m_HoldNearSec + (m_HoldFarSec - m_HoldNearSec) * t;
}


// 指定位置にキャラクターカプセルを置いたときに衝突がないか調べる
bool EnemyAIComponent::IsCapsuleFree(const Vector3& feetPos) const
{
	using namespace JPH;

	if (!m_Physics || !m_Char) { return false; }

	auto& system = m_Physics->GetSystem();
	auto& npq = system.GetNarrowPhaseQuery();

	float halfHeight = m_Char->GetCurrentHalfHeight();
	float radius = m_Char->GetRadius();

	// CharacterVirtualComponent 側から Shape をもらう
	const Shape* capsule = m_Char->GetCurrentShape();
	if (!capsule) { return false; }

	RVec3 center(feetPos.x,
		feetPos.y + halfHeight + radius,
		feetPos.z);
	RMat44 transform = RMat44::sTranslation(center);

	CollideShapeSettings settings;
	RVec3 baseOffset = RVec3::sZero();
	Vec3  scale = Vec3::sReplicate(1.0f);

	auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
	auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);
	EscapeBodyFilter bodyFilter(system);
	ShapeFilter shapeFilter;

	ClosestHitCollisionCollector<CollideShapeCollector> collector;

	npq.CollideShape(
		capsule,
		scale,
		transform,
		settings,
		baseOffset,
		collector,
		bpFilter,
		objFilter,
		bodyFilter,
		shapeFilter
	);

	// 何も当たっていなければ「この位置は安全」
	return !collector.HadHit();
}
