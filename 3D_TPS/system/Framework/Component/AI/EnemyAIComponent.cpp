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

    switch (m_State)
    {
    case EnemyAIComponent::Idle:
		UpdateIdle(dt);
        break;
    case EnemyAIComponent::Patrol:
		UpdatePatrol(dt);
        break;
    case EnemyAIComponent::Investigate:
		UpdateInvestigate(dt);
        break;
    case EnemyAIComponent::Chase:
        break;
    default:
        break;
    }
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
        //Vector3 avoidDir = ComputeAvoidDir(desiredDir);
        
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
    dir.Normalize();

    // Zマイナスが前、という前提は Patrol と同じ
    float yaw = std::atan2(-dir.x, -dir.z);

    Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), yaw);
    m_pOwner->SetRotation(q);
}

void EnemyAIComponent::OnHeardSound(const Vector3& pos, float strength)
{
    // とりあえず最後に聞こえた位置を更新して、調査状態 に入る
    m_LastHeardPosition = pos;

    // すでに追跡中なら無視、などの条件を付けたいならここで分岐
    m_State = Investigate;
    m_InvestigateTimer = 0.0f;
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
    
    if (distToTarget < 1.0f) { return Vector3::Zero; }

    // 本来進みたい「前方」
    Vector3 forward = toTarget / distToTarget;

    // 目の高さあたりからレイを飛ばす
    Vector3 origin3 = pos;
    origin3.y += m_EyeHeight;

    float rayLen = m_RayLength;

    auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);
    BodyFilter bodyFilter;

    auto castDist = [&](const Vector3& dir3) -> float
        {
            Vector3 d = dir3;
            if (d.LengthSquared() < 0.0001f) { return rayLen; }
            d.Normalize();

            RVec3 origin(origin3.x, origin3.y, origin3.z);
            Vec3  jdir(d.x, d.y, d.z);

            RRayCast ray(origin, jdir * rayLen);
            RayCastResult hit;

            if (npq.CastRay(ray, hit, bpFilter, objFilter, bodyFilter))
            {
                // 当たった距離
                return hit.mFraction * rayLen;
            }
            // 何も当たらなければ最大距離まで空いている
            return rayLen;
        };

    // 横方向（水平面の右ベクトル）
    Vector3 side(-forward.z, 0.0f, forward.x);

    // 前・左前・右前への空き具合
    float centerFree = castDist(forward);
    float leftFree = castDist(forward + side);
    float rightFree = castDist(forward - side);

    const float blockThreshold = rayLen * 0.6f; // この距離以内に障害物があれば「塞がれている」

    bool frontBlocked = centerFree < blockThreshold;

    if (!frontBlocked)
    {
        // 前が空いたら回避モード解除して、素直に前進
        m_IsAvoidingWall = false;
        return forward;
    }

    // ここに来るのは「前が壁で塞がれている」ケース

    if (!m_IsAvoidingWall)
    {
        // ★ 回避モードに入る瞬間：左と右のどちらに回り込むか決める
        m_IsAvoidingWall = true;

        // 右側のほうが空いていれば右に回り込む（side に -1）
        m_AvoidSideSign = (rightFree > leftFree) ? -1.0f : 1.0f;
    }

    // 回避モード中：障害物に沿って「横方向＋少し前」の方向へ進む

    Vector3 wallDir = side * m_AvoidSideSign; // 左 or 右
    wallDir.y = 0.0f;
    wallDir.Normalize();

    // 壁に沿って進みつつ、少しだけターゲット方向も混ぜる
    const float alongWallWeight = 0.7f; // 0.7: ほぼ壁沿い、0.3: 目標方向
    const float toTargetWeight = 1.0f - alongWallWeight;

    Vector3 moveDir = wallDir * alongWallWeight + forward * toTargetWeight;

    if (moveDir.LengthSquared() < 0.0001f) { return Vector3::Zero; }

    moveDir.Normalize();
    return moveDir;
}
