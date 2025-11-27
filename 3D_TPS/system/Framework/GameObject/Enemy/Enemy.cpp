#include "Enemy.h"
#include <random>
#include <iostream>
#include "system/Framework/AssetManager/AssetManager.h"
#include "Framework/GameObject/Player/Player.h"
#include "system/Framework/Component/AI/EnemyAIComponent.h"
#include "system/Framework/Component/Physic/CharacterVirtualComponent.h"
#include "system/RandomEngine.h"
#include "Framework/Component/AI/EnemyHearingComponent.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "system/meshmanager.h"

namespace {
	constexpr float ENEMY_CAPSULE_HALFHEIGHT = 60.0f;
	constexpr float ENEMY_CAPSULE_RADIUS = 35.0f;
	constexpr Vector3 ENEMY_COLLIDER_OFFSET = Vector3(0.0f, 80.0f, 0.0f);
}

Enemy::Enemy(ComponentFactory* factory, uint64_t id, const std::string& name, const Tag& tag,
	Player* player,
	const Transform& transform)
	: Character(factory, id, name, tag, transform), m_pPlayer(player)
{
}

Enemy::~Enemy()
{
}


void Enemy::Init(void)
{
    auto& am = AssetManager::GetInstance();

    // ==========
    //  アニメ系
    // ==========
    // 1) コンポーネント追加
    m_pAnimComp = AddComponent<SkinnedAnimationComponent>("SkinnedAnim");

    // 2) メッシュとシェーダ設定（プレイヤーと同じものを使う想定）
    CAnimationMesh* mesh = am.GetAnimationMesh("Akai");
    m_pAnimComp->SetMesh(mesh);

    // MeshManager から共通アニメ用シェーダ取得（プレイヤーと同じ）
    CShader* shader = MeshManager::getShader<CShader>("animshader");
    m_pAnimComp->SetShader(shader);

    // 3) 必要なクリップをキャッシュ
    auto* idle = am.GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
    auto* run = am.GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
    auto* walk = am.GetAnimationData("Walking")->GetAnimation("Walking", 0);

    m_pAnimComp->SetClip(AnimType::Idle, idle);
    m_pAnimComp->SetClip(AnimType::Walk, walk);
    m_pAnimComp->SetClip(AnimType::Run, run);

    // =================================
    //  敵の位置決定（あなたの既存処理）
    // =================================
    auto& rng = RandomEngine::tls();

    float mapMinX = -2500.0f;
    float mapMaxX = 2500.0f;
    float mapMinZ = -2500.0f;
    float mapMaxZ = 2500.0f;

    float startX = static_cast<float>(rng.uniformReal(mapMinX, mapMaxX));
    float startZ = static_cast<float>(rng.uniformReal(mapMinZ, mapMaxZ));

    m_StartPos = Vector3(startX, 5.0f, startZ);
    m_Transform.SetPosition(m_StartPos);

    float patrolRange = 1000.0f;
    float endOffsetX = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
    float endOffsetZ = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
    m_EndPos = m_StartPos + Vector3(endOffsetX, 0.0f, endOffsetZ);

    // テストで固定したいならここを上書き
    m_StartPos = Vector3(500.0f, 0.0f, 0.0f);
    m_EndPos = Vector3(-500.0f, 0.0f, 0.0f);

    // ===========================
    //  物理 / AI / 聴覚コンポーネント
    // ===========================

    // 1) CharacterVirtualComponent
    {
        m_CharComp = AddComponent<CharacterVirtualComponent>("EnemyCharacter");
        m_CharComp->SetCapsule(ENEMY_CAPSULE_HALFHEIGHT, ENEMY_CAPSULE_RADIUS);
        m_CharComp->SetOffset(ENEMY_COLLIDER_OFFSET);
    }

    // 2) EnemyAIComponent
    {
        m_AIComp = AddComponent<EnemyAIComponent>("EnemyAI");

        std::vector<Vector3> waypoints;
        waypoints.reserve(2);
        waypoints.push_back(m_StartPos);
        waypoints.push_back(m_EndPos);

        m_AIComp->SetWayPoints(waypoints);
        m_AIComp->SetArriveRadius(50.0f);
        m_AIComp->SetRayLength(300.0f);
        m_AIComp->SetAvoidWeight(1.5f);
        m_AIComp->SetEyeHeight(80.0f);
    }

    // 3) EnemyHearingComponent
    {
        auto hearingComp = AddComponent<EnemyHearingComponent>("EnemyHearing");
        hearingComp->SetEnemyAI(m_AIComp);
    }

    GameObject::Init();
}

void Enemy::Update(const float deltatime)
{
    if (m_pPlayer && m_pPlayer->IsDestroy())
    {
        m_pPlayer = nullptr;
    }

    // まずコンポーネント更新（AI / CharacterVirtual など）
    GameObject::Update(deltatime);

    // ==========
    //  アニメ判定
    // ==========
    Vector3 vel = Vector3::Zero;
    if (m_CharComp)
    {
        vel = m_CharComp->GetLinearVelocity();
        // 垂直速度は無視して水平速度だけ見たいなら
        // vel.y = 0.0f;
    }

    if (m_pAnimComp)
    {
        float speed = vel.Length();

        // 閾値は好みで調整
        const float walkThreshold = 10.0f;   // これ未満なら Idle
        const float runThreshold = 200.0f;  // これ以上なら Run

        if (speed < walkThreshold)
        {
            // ほぼ停止 → Idle
            m_pAnimComp->Play(AnimType::Idle, 0.1f);
        }
        else if (speed < runThreshold)
        {
            // そこそこ動いている → Walk
            m_pAnimComp->Play(AnimType::Walk, 0.1f);
        }
        else
        {
            // だいぶ速い → Run
            m_pAnimComp->Play(AnimType::Run, 0.1f);
        }
    }
}


void Enemy::Draw(void) const
{
    Character::Draw();
}

void Enemy::Uninit(void)
{
	Character::Uninit();
}

bool Enemy::CanSeePlayer(const Vector3& playerPos) const
{
	Vector3 enemyPos = m_Transform.GetPosition();

	// 前方向ベクトル
	Vector3 forward = Vector3::TransformNormal(Vector3(0, 0, -1),
		Matrix4x4::CreateFromQuaternion(m_Transform.GetRotation()));

	// プレイヤーへのベクトル（高さを無視）
	Vector3 toPlayer = playerPos - enemyPos;
	toPlayer.y = 0; // 水平面だけで判定
	float distance = toPlayer.Length();
	if (distance > m_ViewDistance) return false;

	toPlayer.Normalize();
	forward.y = 0;
	forward.Normalize();

	// 内積で角度判定
	float dot = std::clamp(forward.Dot(toPlayer), -1.0f, 1.0f);
	float angle = std::acos(dot) * (180.0f / 3.14159f);

	return angle <= (m_ViewAngle * 0.5f);
}