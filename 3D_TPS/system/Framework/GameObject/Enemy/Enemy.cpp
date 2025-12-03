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
#include "system/Framework/Time/Time.h"
#include "system/imgui/imgui.h"
#include "system/DebugUI.h"

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
	//auto* surprise_right = am.GetAnimationData("Surprise_RightTurn")->GetAnimation("Surprise_RightTurn", 0);
	//auto* surprise_left = am.GetAnimationData("Surprise_LeftTurn")->GetAnimation("Surprise_LeftTurn", 0);
	auto* surprise_right = am.GetAnimationData("Right_Turn")->GetAnimation("Right_Turn", 0);
	auto* surprise_left = am.GetAnimationData("Left_Turn")->GetAnimation("Left_Turn", 0);

	m_pAnimComp->SetClip(AnimType::Idle, idle);
	m_pAnimComp->SetClip(AnimType::Walk, walk);
	m_pAnimComp->SetClip(AnimType::Run, run);
	m_pAnimComp->SetClip(AnimType::Surprise_RightTurn, surprise_right);
	m_pAnimComp->SetClip(AnimType::Surprise_LeftTurn, surprise_left);

	// =================================
	//  敵の位置決定（あなたの既存処理）
	// =================================
	auto& rng = RandomEngine::tls();

	float mapMinX = -2500.0f;
	float mapMaxX = 2500.0f;
	float mapMinZ = -2500.0f;
	float mapMaxZ = 2500.0f;

	//float startX = static_cast<float>(rng.uniformReal(mapMinX, mapMaxX));
	//float startZ = static_cast<float>(rng.uniformReal(mapMinZ, mapMaxZ));

	//m_StartPos = Vector3(startX, 5.0f, startZ);
	//m_Transform.SetPosition(m_StartPos);

	float patrolRange = 1000.0f;
	float endOffsetX = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
	float endOffsetZ = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
	m_EndPos = m_StartPos + Vector3(endOffsetX, 0.0f, endOffsetZ);

	// テストで固定したいならここを上書き
	m_StartPos = Vector3(-300.0f, 10.0f, 1750.0f);
	m_EndPos = Vector3(-300.0f, 10.0f, 140.0f);
	//m_StartPos = Vector3(500.0f, 0.0f, 0.0f);
	//m_EndPos = Vector3(-500.0f, 0.0f, 0.0f);

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
		//m_AIComp->SetArriveRadius(50.0f);
		m_AIComp->SetRayLength(300.0f);
		m_AIComp->SetAvoidWeight(1.5f);
		m_AIComp->SetEyeHeight(80.0f);
		m_AIComp->SetPlayer(m_pPlayer);
	}

	// 3) EnemyHearingComponent
	{
		auto hearingComp = AddComponent<EnemyHearingComponent>("EnemyHearing");
		hearingComp->SetEnemyAI(m_AIComp);
	}

	DebugUI::RedistDebugFunction([this]() {
		DebugImGui();
		});
}

void Enemy::Update(const float deltatime)
{
	if (m_pPlayer && m_pPlayer->IsDestroy())
	{
		m_pPlayer = nullptr;
	}

	// 1) まずコンポーネント更新（AI / CharacterVirtual など）
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

	// ========== プレイヤー発見 → スロー＋ゲームオーバー ==========
	if (!m_GameOverTriggered && m_AIComp && m_AIComp->IsFound())
	{
		m_GameOverTriggered = true;

		// タイムスケールを 0.5 に
		Time::GetInstance().SetTimeScale(0.5f);

		// ゲームオーバー遷移
		// 実際の実装に合わせて書き換える
		//auto& sm = SceneManager::GetInstance();
		//sm.ChangeScene;  // もしくは sm.ChangeScene(SceneType::GameOver);
	}

	// ========== アニメ判定 ==========
	if (m_pAnimComp)
	{
		if (aiState == EnemyAIComponent::State::Caution && m_AIComp)
		{
			// 1) Caution かつ「振り向き中」のあいだは、
			//    TryStartSurpriseTurn で再生した驚きアニメを維持するだけ。
			if (m_AIComp->IsInCautionTurnPhase())
			{
				// ここでは何も再生しない
				// → 最初にかけた Surprise_Left/RightTurn がそのまま流れ続ける
			}
			else
			{
				// 2) 振り向きが終わって「待機時間」に入ったら Idle に切り替える
				m_pAnimComp->Play(AnimType::Idle, 0.1f);
			}

			// Caution 中はここで終了。歩き/走りアニメには切り替えない
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

	// todo : アニメーションだけ逆で設定してるので注意
	if (turnRight)
	{
		m_pAnimComp->Play(AnimType::Surprise_LeftTurn, 0.1f);
	}
	else
	{
		m_pAnimComp->Play(AnimType::Surprise_RightTurn, 0.1f);
	}

	return true;
}


void Enemy::DebugImGui(void)
{
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
}