#include "Enemy.h"
#include <random>
#include <iostream>
#include "system/Framework/AssetManager/AssetManager.h"
#include "Framework/GameObject/Player/Player.h"
#include "Framework/GameObject/Terrain/Terrain.h"
#include "system/Framework/Component/AI/EnemyAIComponent.h"
#include "system/Framework/Component/Physic/CharacterVirtualComponent.h"
#include "system/RandomEngine.h"
#include "Framework/Component/AI/EnemyHearingComponent.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "system/meshmanager.h"
#include "system/Framework/Time/Time.h"
#include "system/imgui/imgui.h"
#include "system/DebugUI.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"

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
	auto& rng = RandomEngine::tls();

	// 1) 見た目・アニメ周りの初期化
	InitAnimation(am);

	// 2) 地形に沿った巡回点の決定
	InitPatrolPoints(rng);

	// 3) 物理・AI・聴覚コンポーネントの初期化
	InitComponents();

	DebugUI::RedistDebugFunction([this]() {
		DebugImGui();
		});
}

// ----------------------------------------
// 1) アニメ関連
// ----------------------------------------
void Enemy::InitAnimation(AssetManager& am)
{
	// アニメーションコンポーネント追加
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("SkinnedAnim");

	// メッシュ・シェーダ設定
	CAnimationMesh* mesh = am.GetAnimationMesh("Akai");
	m_pAnimComp->SetMesh(mesh);

	CShader* shader = MeshManager::getShader<CShader>("animshader");
	m_pAnimComp->SetShader(shader);

	// クリップ取得
	auto* idle = am.GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
	auto* run = am.GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
	auto* walk = am.GetAnimationData("Walking")->GetAnimation("Walking", 0);
	auto* right = am.GetAnimationData("Right_Turn")->GetAnimation("Right_Turn", 0);
	auto* left = am.GetAnimationData("Left_Turn")->GetAnimation("Left_Turn", 0);

	// 種類ごとに登録
	m_pAnimComp->SetClip(AnimType::Idle, idle);
	m_pAnimComp->SetClip(AnimType::Walk, walk);
	m_pAnimComp->SetClip(AnimType::Run, run);
	m_pAnimComp->SetClip(AnimType::Surprise_RightTurn, right);
	m_pAnimComp->SetClip(AnimType::Surprise_LeftTurn, left);
}

// ----------------------------------------
// 2) 巡回点決定（地形コライダからサンプリング）
// ----------------------------------------
void Enemy::InitPatrolPoints(RandomEngine& rng)
{
	// 地形が設定されていない場合のフォールバック
	auto SetDefaultPatrol = [&]() {
		m_StartPos = Vector3(-300.0f, 210.0f, 1750.0f);
		m_EndPos = Vector3(-300.0f, 210.0f, -540.0f);
		};

	//if (!m_pTerrain)
	//{
	//	SetDefaultPatrol();
	//	m_Transform.SetPosition(m_StartPos);
	//	return;
	//}

	//// StaticMeshCollider から AABB と高さサンプルを取る
	//StaticMeshCollider* terrainCol = m_pTerrain->GetComponent<StaticMeshCollider>();
	//if (!terrainCol)
	//{
	//	SetDefaultPatrol();
	//	m_Transform.SetPosition(m_StartPos);
	//	return;
	//}

	//Vector3 xzMin, xzMax;
	//if (!terrainCol->GetWorldXZBounds(xzMin, xzMax))
	//{
	//	SetDefaultPatrol();
	//	m_Transform.SetPosition(m_StartPos);
	//	return;
	//}

	//// 地形範囲内のランダム XZ を生成
	//auto RandXZInTerrain = [&]() -> Vector3 {
	//	float x = static_cast<float>(rng.uniformReal(xzMin.x, xzMax.x));
	//	float z = static_cast<float>(rng.uniformReal(xzMin.z, xzMax.z));
	//	return Vector3(x, 0.0f, z);
	//	};

	//constexpr int   MAX_TRY = 16;
	//constexpr float HEIGHT_OFFSET = 5.0f;

	//bool ok = false;
	//for (int i = 0; i < MAX_TRY && !ok; ++i)
	//{
	//	Vector3 p0 = RandXZInTerrain();
	//	Vector3 p1 = RandXZInTerrain();

	//	float y0, y1;
	//	if (terrainCol->SampleHeight(p0.x, p0.z, y0) &&
	//		terrainCol->SampleHeight(p1.x, p1.z, y1))
	//	{
	//		m_StartPos = Vector3(p0.x, y0 + HEIGHT_OFFSET, p0.z);
	//		m_EndPos = Vector3(p1.x, y1 + HEIGHT_OFFSET, p1.z);
	//		ok = true;
	//	}
	//}

	//if (!ok)
	//{
	//	SetDefaultPatrol();
	//}

	SetDefaultPatrol();

	// 実際の Transform を開始地点に合わせる
	m_Transform.SetPosition(m_StartPos);
}

// ----------------------------------------
// 3) 物理 / AI / 聴覚コンポーネント
// ----------------------------------------
void Enemy::InitComponents()
{
	// CharacterVirtualComponent
	{
		m_CharComp = AddComponent<CharacterVirtualComponent>("EnemyCharacter");
		m_CharComp->SetCapsule(ENEMY_CAPSULE_HALFHEIGHT, ENEMY_CAPSULE_RADIUS);
		m_CharComp->SetOffset(ENEMY_COLLIDER_OFFSET);
	}

	// EnemyAIComponent
	{
		m_AIComp = AddComponent<EnemyAIComponent>("EnemyAI");

		std::vector<Vector3> waypoints;
		waypoints.reserve(2);
		waypoints.push_back(m_StartPos);
		waypoints.push_back(m_EndPos);

		m_AIComp->SetWayPoints(waypoints);
		m_AIComp->SetRayLength(900.0f);
		m_AIComp->SetAvoidWeight(1.5f);
		m_AIComp->SetEyeHeight(80.0f);
		m_AIComp->SetPlayer(m_pPlayer);
	}

	// EnemyHearingComponent
	{
		auto hearingComp = AddComponent<EnemyHearingComponent>("EnemyHearing");
		hearingComp->SetEnemyAI(m_AIComp);
	}
}

void Enemy::Update(const float deltatime)
{
	if (m_pPlayer && m_pPlayer->IsDestroy())
	{
		m_pPlayer = nullptr;
	}

	// ==============================
	// すでにプレイヤー発見演出中なら
	//   ・移動更新を止める（GameObject::Update を呼ばない）
	//   ・Idle アニメだけ流す
	// ==============================
	if (m_GameOverTriggered)
	{
		// 念のため毎フレ入力も 0 にしておく
		if (m_CharComp)
		{
			m_CharComp->SetMoveDir(Vector3::Zero);
		}

		if (m_pAnimComp)
		{
			m_pAnimComp->Play(AnimType::Idle, 0.1f);
		}

		return;
	}

	// 1) コンポーネント更新（AI / CharacterVirtual / Animator など）
	GameObject::Update(deltatime);

	// 2) 今フレーム「音を聞いた」通知が AI に入っていれば、驚きアニメ開始
	if (m_AIComp)
	{
		Vector3 heardPos;
		if (m_AIComp->ConsumeHeardSoundPosition(heardPos))
		{
			TryStartSurpriseTurn(heardPos);
		}
	}

	// ==== ここで AI のステートを取得 ====
	EnemyAIComponent::State aiState = EnemyAIComponent::State::Idle;
	if (m_AIComp)
	{
		aiState = m_AIComp->GetState();
	}

	// ==============================
	// 3) プレイヤー発見チェック
	// ==============================
	if (m_AIComp && m_AIComp->IsFound())
	{
		OnFoundPlayer();      // ★ ここで m_GameOverTriggered が true になる
		// このフレームの残り処理はそのまま進むが、
		// 次フレームからは上の if(m_GameOverTriggered) で止まる
	}

	// ==============================
	// 4) アニメ判定
	// ==============================
	if (m_pAnimComp)
	{
		if (aiState == EnemyAIComponent::State::Caution && m_AIComp)
		{
			if (m_AIComp->IsInCautionTurnPhase())
			{
				// 振り向きアニメを維持
			}
			else
			{
				// 待機フェーズは Idle
				m_pAnimComp->Play(AnimType::Idle, 0.1f);
			}
			return;
		}

		// それ以外の状態: Idle / Walk / Run を速度に応じて再生
		Vector3 vel = Vector3::Zero;
		if (m_CharComp)
		{
			vel = m_CharComp->GetLinearVelocity();
			// 必要なら vel.y = 0.0f;
		}

		float speed = vel.Length();

		const float walkThreshold = 10.0f;
		const float runThreshold = 200.0f;

		if (speed < walkThreshold)
		{
			m_pAnimComp->Play(AnimType::Idle, 0.1f);
		}
		else if (speed < runThreshold)
		{
			m_pAnimComp->Play(AnimType::Walk, 0.1f);
		}
		else
		{
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
	return m_AIComp ? m_AIComp->CanSeePlayer() : false;
}

bool Enemy::TryStartSurpriseTurn(const Vector3& soundPos)
{
	if (!m_pAnimComp)
		return false;

	Vector3 pos = m_Transform.GetPosition();
	Vector3 toSound = soundPos - pos;
	toSound.y = 0.0f;

	if (toSound.LengthSquared() < 1e-4f)
		return false;

	toSound.Normalize();

	// 現在の前方（Z+ 前提）
	Vector3 forward = m_Transform.GetForward();
	forward.y = 0.0f;
	if (forward.LengthSquared() < 1e-4f)
	{
		forward = Vector3::Forward; // (0,0,1)
	}
	forward.Normalize();

	// forward × toSound の Y 成分で左右判定
	float crossY = forward.x * toSound.z - forward.z * toSound.x;

	// ここは実際の見た目に合わせて決める
	bool turnRight = (crossY > 0.0f);  // 右向きアニメかどうか

	// 回転アニメ再生
	if (turnRight)
	{
		m_pAnimComp->Play(AnimType::Surprise_RightTurn, 0.1f);
	}
	else
	{
		m_pAnimComp->Play(AnimType::Surprise_LeftTurn, 0.1f);
	}

	return true;
}

void Enemy::OnFoundPlayer(void)
{
	// すでに演出中なら何もしない
	if (m_GameOverTriggered)
		return;

	m_GameOverTriggered = true;

	// 1) 移動停止
	if (m_CharComp)
	{
		m_CharComp->Stop();
	}

	// 2) プレイヤーの方向を向く
	if (m_pPlayer)
	{
		Vector3 selfPos = m_Transform.GetPosition();
		Vector3 playerPos = m_pPlayer->GetTransform().GetPosition();

		Vector3 dir = playerPos - selfPos;
		dir.y = 0.0f;

		if (dir.LengthSquared() > 1e-4f)
		{
			dir.Normalize();
			// モデルが Z- 前方系なので FaceMoveDir と同じ計算
			float yaw = std::atan2(-dir.x, -dir.z);
			Quaternion q = Quaternion::CreateFromAxisAngle(Vector3::Up, yaw);
			m_Transform.SetRotation(q);
		}
	}

	// 3) スローモーション演出
	Time::GetInstance().SetTimeScale(0.5f);
}



void Enemy::DebugImGui(void)
{
	// Enemy 基本情報
	if (ImGui::CollapsingHeader("Enemy Transform"))
	{
		Vector3 pos = m_Transform.GetPosition();
		Vector3 scale = m_Transform.GetScale();

		ImGui::Text("Pos:   (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		ImGui::Text("Scale: (%.2f, %.2f, %.2f)", scale.x, scale.y, scale.z);

		Matrix4x4 world = m_Transform.GetWorldMatrix();
		Vector3 worldScale;
		Quaternion worldRot;
		Vector3 worldPos;
		if (world.Decompose(worldScale, worldRot, worldPos))
		{
			ImGui::Text("WorldScale: (%.2f, %.2f, %.2f)",
				worldScale.x, worldScale.y, worldScale.z);
		}
	}
	// プレイヤー基本情報
	if (m_pPlayer && ImGui::CollapsingHeader("Player Transform"))
	{
		Vector3 pos = m_pPlayer->GetTransform().GetPosition();
		Vector3 scale = m_pPlayer->GetTransform().GetScale();
		ImGui::Text("Pos:   (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		ImGui::Text("Scale: (%.2f, %.2f, %.2f)", scale.x, scale.y, scale.z);
	}
}