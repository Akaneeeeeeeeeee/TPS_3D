#include "EnemyAIComponent.h"
#include "Framework/Component/Physic/CharacterVirtualComponent.h"
#include "Framework/GameObject/GameObject.h"
#include "Framework/GameObject/Player/Player.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>

namespace
{
    constexpr float DEG2RAD = PI / 180.0f;
}


void EnemyAIComponent::Attach(EngineContext& ctx)
{
    m_Physics = &ctx.joltPhysicsManager;
    m_Char = m_pOwner->GetComponent<CharacterVirtualComponent>();
}

void EnemyAIComponent::Detach(void)
{
    m_Physics = nullptr;
    m_Char = nullptr;
}

void EnemyAIComponent::Init(void)
{
	// キャラクターコンポーネントの取得
    if (!m_Char)
    { 
        m_Char = m_pOwner->GetComponent<CharacterVirtualComponent>(); 
    }

}

void EnemyAIComponent::Update(const float dt)
{
    if (!m_Char) { return; }

    // 1) 状態ごとの処理（Caution の中で視線補間）
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
    case EnemyAIComponent::Chase:
        UpdateChase(dt);
        break;
    default:
        break;
    }

    // 2) Caution 以外は本体の forward を視線にコピー
    if (m_State != State::Caution && m_pOwner)
    {
        m_ViewForward = m_pOwner->GetForward(); // Z+ 前方系
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

    // 3) 最終的な m_ViewForward を使って視線チェック
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
    Vector3 target = m_WayPoints[m_CurrentIndex];

    Vector3 toTarget = target - pos;
    float distSq = toTarget.LengthSquared();

    // 目標地点に十分近づいたら次のウェイポイントへ
    if (distSq < m_ArriveRadius * m_ArriveRadius)
    {
        m_CurrentIndex = (m_CurrentIndex + 1) % m_WayPoints.size();
        target = m_WayPoints[m_CurrentIndex];
        toTarget = target - pos;
        distSq = toTarget.LengthSquared();
    }
    
    // 障害物回避
    Vector3 moveDir = ComputeMoveDirToTarget(target);

    if (moveDir.LengthSquared() > 0.0001f)
    {
        m_Char->SetMoveDir(moveDir);
        FaceMoveDir(moveDir);
    }
    else
    {
        m_Char->SetMoveDir(Vector3::Zero);
    }
}

void EnemyAIComponent::UpdateInvestigate(const float dt)
{
    Vector3 pos = m_pOwner->GetPosition();
    Vector3 toTarget = m_LastHeardPosition - pos;
    float distSq = toTarget.LengthSquared();

    // 1) まだ音源位置に到達していない → そこに向かって移動
    if (distSq > m_ArriveRadius * m_ArriveRadius)
    {
        m_InvestigateTimer = 0.0f; // 移動中はタイマーリセット

        toTarget.Normalize();

        Vector3 desiredDir = toTarget;
        
        Vector3 moveDir = ComputeMoveDirToTarget(m_LastHeardPosition);

        if (moveDir.LengthSquared() > 0.0001f)
        {
			// 正規化してセット
            moveDir.Normalize();
            m_Char->SetMoveDir(moveDir);
			// キャラの向きも合わせる
            FaceMoveDir(moveDir);
        }
        else
        {
            m_Char->SetMoveDir(Vector3::Zero);
        }

        return;
    }

    // 2) 音源位置付近に着いた → しばらく様子を見る
    m_Char->SetMoveDir(Vector3::Zero);
    m_InvestigateTimer += dt;

    if (m_InvestigateTimer >= m_InvestigateWaitTime)
    {
        // 一定時間経過したら巡回に戻る
        m_InvestigateTimer = 0.0f;

        if (!m_WayPoints.empty())
        {
            // 近いウェイポイントを探してそこから再開してもいいし、
            // 今のインデックスのまま戻ってもいい
            m_State = Patrol;
        }
        else
        {
            m_State = Idle;
        }
    }
}

// 追跡状態の更新(現状発見されたらゲームオーバーなので、追跡が必要になったら実装)
void EnemyAIComponent::UpdateChase(const float deltatime)
{
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
    // 驚いてる最中は方向だけ更新する
    if (m_State == State::Caution)
    {
        //m_LastHeardPosition = pos;
        return;
    }

    // Idle / Patrol / Investigate / Chase から来た場合は
    // 「新しい驚き」として扱う
    m_LastHeardPosition = pos;
    m_HeardThisFrame = true;   // Enemy が驚きアニメを再生するトリガ
    if (!m_pOwner) return;

    // 角度チェックは必要なら残す（正面ほぼ一致なら直接 Investigate へ）
    Vector3 selfPos = m_pOwner->GetPosition();
    Vector3 toSound = pos - selfPos;
    toSound.y = 0.0f;

    if (toSound.LengthSquared() < 1e-4f)
    {
        m_State = Investigate;
        m_InvestigateTimer = 0.0f;
        return;
    }
    toSound.Normalize();

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
    if (!m_pPlayer || !m_Physics || !m_pOwner)
        return;

    if (CanSeePlayer())
    {
        m_IsFound = true;
        m_LastHeardPosition = m_pPlayer->GetPosition(); // 視覚でも最後に見た位置を覚えておく

        // 追跡ステートに移るならここ
        // m_State = Chase;
    }
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
    Vector3 toTarget = targetPos - eyePos;
    float   dist = toTarget.Length();
    if (dist <= 0.0001f || dist > m_ViewDistance)
        return false;

    Vector3 dir = toTarget / dist;

    Vector3 forward = GetViewForward();
    forward.Normalize();

    float cosAngle = forward.Dot(dir);
    float cosHalfFov = std::cos(m_ViewAngle * 0.5f * DEG2RAD);

    return cosAngle >= cosHalfFov;
}

bool EnemyAIComponent::CanSeePoint(const Vector3& eyePos, const Vector3& targetPos) const
{
    using namespace JPH;
    if (!m_Physics) return false;

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
        return Vector3::Zero;
    }

    Vector3 forward = toTarget / distToTarget;

    Vector3 origin3 = pos;
    origin3.y += m_EyeHeight;

    float rayLen = m_RayLength;
    float maxCheckDist = std::min(rayLen, distToTarget);
    //float blockThreshold = maxCheckDist * 0.5f;
    float blockThreshold = maxCheckDist;

    // レイの持ち主は「キャラ」なので CHARACTER を渡す
    auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);

    // キャラ／トリガー（必要なら MOVING も）を無視する BodyFilter
    AvoidCharAndTriggerBodyFilter bodyFilter(system);

    auto castDist = [&](const Vector3& dir3) -> float
        {
            Vector3 d = dir3;
            if (d.LengthSquared() < 0.0001f) { return maxCheckDist; }
            d.Normalize();

            RVec3 origin(origin3.x, origin3.y, origin3.z);
            Vec3  jdir(d.x, d.y, d.z);

            RRayCast ray(origin, jdir * maxCheckDist);
            RayCastResult hit;

            if (npq.CastRay(ray, hit, bpFilter, objFilter, bodyFilter))
            {
                return hit.mFraction * maxCheckDist;
            }
            return maxCheckDist;
        };

    Vector3 side(-forward.z, 0.0f, forward.x);
    if (side.LengthSquared() < 0.0001f)
        side = Vector3(1, 0, 0);

    float centerFree = castDist(forward);
    float leftFree = castDist(forward + side);
    float rightFree = castDist(forward - side);

    bool frontBlocked = centerFree < blockThreshold;

    if (!frontBlocked)
    {
        m_IsAvoidingWall = false;
        return forward;
    }

    if (!m_IsAvoidingWall)
    {
        m_IsAvoidingWall = true;
        m_AvoidSideSign = (rightFree > leftFree) ? -1.0f : 1.0f;
    }

    Vector3 moveDir = side * m_AvoidSideSign;
    moveDir.y = 0.0f;

    if (moveDir.LengthSquared() < 0.0001f)
    {
        moveDir = forward;
    }

    moveDir.Normalize();
    return moveDir;
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
        if (t > 1.0f) t = 1.0f;

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
    }
}
