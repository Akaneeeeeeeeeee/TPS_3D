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
#include "Framework/Component/StateIcon/EnemyHeadIconComponent.h"
#include "system/meshmanager.h"
#include "system/Framework/Time/Time.h"
#include "system/imgui/imgui.h"
#include "system/DebugUI.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/Component/Renderer/MeshRenderer/SkinnedMeshRendererComponent.h"
#include "Framework/Component/Renderer/SpriteRenderer/UISpriteRenderer.h"

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

void Enemy::SetWayPoints(const std::vector<Vector3>& points)
{
	m_RequestedWayPoints = points;

	if (m_AIComp)
	{
		m_AIComp->SetWayPoints(m_RequestedWayPoints);
	}
}


void Enemy::Awake(void)
{
	auto& rng = RandomEngine::tls();

	// 1) 見た目・アニメ周りの初期化
	InitAnimation();

	// 2) 物理・AI・聴覚コンポーネントの初期化
	InitComponents();

	// 外部指定の巡回点があるなら、AI生成後にここで反映
	if (m_AIComp && !m_RequestedWayPoints.empty())
	{
		m_AIComp->SetWayPoints(m_RequestedWayPoints);
	}

	DebugUI::RedistDebugFunction([this]() {
		DebugImGui();
		});
}

void Enemy::Start(void)
{
	// 1) AI用の Player* は「本編でだけ」取れれば使う（タイトルでは null のまま）
	if (!m_pPlayer && m_pObjectManager)
	{
		// ※タイトルの Tag::Player は TitlePlayerActor なので、ここは null になってOK
		m_pPlayer = m_pObjectManager->GetObjectByTag<Player>(Tag::Player);
	}

	// 2) HeadIcon 用カメラは「Tag::Player の Character から CameraComponent を拾う」
	CameraComponent* cam = nullptr;

	if (m_pPlayer)
	{
		cam = m_pPlayer->GetCamera(); // 本編
	}
	else if (m_pObjectManager)
	{
		// タイトル：TitlePlayerActor も Character なので拾える
		if (auto* playerCh = m_pObjectManager->GetObjectByTag<Character>(Tag::Player))
		{
			cam = playerCh->GetComponent<CameraComponent>();
		}
	}

	if (m_HeadIcon && cam)
	{
		m_HeadIcon->Setup(cam, 64, 64,
			"assets/texture/hatena-illust1.png",
			"assets/texture/b-mk-illust2.png");
		m_HeadIcon->SetOffset(Vector3(0, 200, 0));
		m_HeadIcon->SetScale(Vector3(1, 1, 1));
	}

	// 3) AIには Player* がある時だけ渡す（タイトルは視認なしでOK）
	if (m_AIComp && m_pPlayer)
		m_AIComp->SetPlayer(m_pPlayer);

	// 4) 地形コライダ
	if (m_AIComp && m_pTerrain)
	{
		m_pTerrainCollider = m_pTerrain->GetComponent<StaticMeshCollider>();
		m_AIComp->SetTerrainCollider(m_pTerrainCollider);
	}
	// すでに巡回点が設定されている場合はスキップ
	if (!m_AIComp->GetWayPoints().empty()) { return; }

	// 3) 巡回点の初期化
	auto& rng = RandomEngine::tls();
	InitPatrolPoints(rng);

	// AIに巡回点を反映
	if (m_AIComp)
	{
		std::vector<Vector3> waypoints;
		waypoints.reserve(2);
		waypoints.push_back(m_StartPos);
		waypoints.push_back(m_EndPos);
		m_AIComp->SetWayPoints(waypoints);
	}
}

// ----------------------------------------
// 1) アニメ関連
// ----------------------------------------
void Enemy::InitAnimation(void)
{
	// アニメーションコンポーネント追加
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("SkinnedAnim");

	SkinnedAnimSetup setup{};
	setup.meshName = "Solider";
	setup.shaderName = "animshader";

	setup.clips = {
		{ AnimType::Idle,               "Solider_Idle",    "Solider_Idle",    0, 1.0f },
		{ AnimType::Walk,               "Solider_Walking", "Solider_Walking",      0, 1.0f },
		{ AnimType::Run,                "Solider_Run",     "Solider_Run",     0, 1.0f },
		{ AnimType::Surprise_RightTurn, "Right_Turn",   "Right_Turn",   0, 1.0f },
		{ AnimType::Surprise_LeftTurn,  "Left_Turn",    "Left_Turn",    0, 1.0f },
		{ AnimType::LookAround,         "LookAround",   "LookAround",   0, 1.0f },
		{ AnimType::GunShot,			"GunShot",		"GunShot",		0, 1.0f },
	};

	m_pAnimComp->SetupFromAssets(setup);

	// 描画（RenderManagerに出す）
	auto* r = AddComponent<SkinnedMeshRendererComponent>("SkinnedRenderer");
	r->SetMeshKey("Solider");
	r->SetShaderKey("animshader");
	r->SetAnimator(m_pAnimComp);
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

	// StaticMeshCollider から AABB と高さサンプルを取る
	m_pTerrainCollider = m_pTerrain->GetComponent<StaticMeshCollider>();
	if (!m_pTerrainCollider)
	{
		SetDefaultPatrol();
		m_Transform.SetPosition(m_StartPos);
		return;
	}

//#ifdef _DEBUG
//	SetDefaultPatrol();
//#else
	Vector3 xzMin, xzMax;
	if (!m_pTerrainCollider->GetWorldXZBounds(xzMin, xzMax))
	{
		SetDefaultPatrol();
		m_Transform.SetPosition(m_StartPos);
		return;
	}

	// 地形範囲内のランダム XZ を生成
	auto RandXZInTerrain = [&]() -> Vector3 {
		float x = static_cast<float>(rng.uniformReal(xzMin.x, xzMax.x));
		float z = static_cast<float>(rng.uniformReal(xzMin.z, xzMax.z));
		return Vector3(x, 0.0f, z);
		};

	constexpr int   MAX_TRY = 16;
	constexpr float HEIGHT_OFFSET = 75.0f;

	bool ok = false;
	for (int i = 0; i < MAX_TRY && !ok; ++i)
	{
		Vector3 p0 = RandXZInTerrain();
		Vector3 p1 = RandXZInTerrain();

		float y0, y1;
		if (m_pTerrainCollider->SampleHeight(p0.x, p0.z, y0) &&
			m_pTerrainCollider->SampleHeight(p1.x, p1.z, y1))
		{
			m_StartPos = Vector3(p0.x, y0 + HEIGHT_OFFSET, p0.z);
			m_EndPos = Vector3(p1.x, y1 + HEIGHT_OFFSET, p1.z);
			ok = true;
		}
	}

	if (!ok)
	{
		SetDefaultPatrol();
	}
//#endif

	// 実際の Transform を開始地点に合わせる
	m_Transform.SetPosition(m_StartPos);
}

// ----------------------------------------
// 3) 物理 / AI / 聴覚コンポーネント
// ----------------------------------------
void Enemy::InitComponents()
{
	// EnemyAIComponent
	{
		m_AIComp = AddComponent<EnemyAIComponent>("EnemyAI");

		m_AIComp->SetRayLength(900.0f);
		m_AIComp->SetAvoidWeight(1.5f);
		m_AIComp->SetEyeHeight(80.0f);
	}

	// CharacterVirtualComponent
	{
		m_CharComp = AddComponent<CharacterVirtualComponent>("EnemyCharacter");
		m_CharComp->SetCapsule(ENEMY_CAPSULE_HALFHEIGHT, ENEMY_CAPSULE_RADIUS);
		m_CharComp->SetOffset(ENEMY_COLLIDER_OFFSET);
	}
	
	// EnemyHearingComponent
	{
		auto hearingComp = AddComponent<EnemyHearingComponent>("EnemyHearing");
		hearingComp->SetEnemyAI(m_AIComp);
	}

	// EnemyHeadIconComponent + BillboardSpriteRenderer
	{
		m_HeadIcon = AddComponent<EnemyHeadIconComponent>("HeadIcon");

		m_UISprite = AddComponent<UISpriteRenderer>("HeadIconRenderer");
		m_UISprite->SetPhase(RenderPhase::OverlayWorld);
		m_UISprite->SetSource(m_HeadIcon);
		m_UISprite->SetDepthTest(false);
		m_UISprite->SetLayer(0);
		m_UISprite->SetOrder(0);
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
		if (m_CharComp) m_CharComp->SetMoveDir(Vector3::Zero);

		if (!m_pAnimComp)
		{
			// アニメが無い場合の保険：即遷移
			m_RequestSceneTransition = true;
			return;
		}

		if (!m_ShotStarted)
		{
			m_ShotStarted = true;

			// 非ループで射撃開始（確実に 0 秒から）
			m_pAnimComp->ForceSet(AnimType::GunShot, 0.0f, false);
			return;
		}

		// GunShot が「現在再生中」かつ「終了した」なら遷移OK
		if (m_pAnimComp->IsPlaying(AnimType::GunShot) && m_pAnimComp->IsCurrentFinished())
		{
			m_RequestSceneTransition = true;
		}
		return;
	}

	// 2) 今フレーム「音を聞いた」通知が AI に入っていれば、驚きアニメ開始
	bool startedSoundTurn = false;
	if (m_AIComp)
	{
		Vector3 heardPos;
		if (m_AIComp->ConsumeHeardSoundPosition(heardPos))
		{
			startedSoundTurn = TryStartSurpriseTurn(heardPos);
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
		OnFoundPlayer();
		return;
		// このフレームの残り処理はそのまま進むが、
		// 次フレームからは上の if(m_GameOverTriggered) で止まる
	}

	// ==============================
	// 4) アニメ判定
	// ==============================
	if (m_pAnimComp)
	{
		// ---- Caution（怪しんでる） ----
		if (aiState == EnemyAIComponent::State::Caution)
		{
			// Cautionに入った瞬間（視覚の可能性が高い）に LookAround を開始
			if (m_PrevAIState != EnemyAIComponent::State::Caution)
			{
				if (!startedSoundTurn)
				{
					m_pAnimComp->Play(AnimType::LookAround, 0.1f);
				}
			}

			if (m_AIComp->IsInCautionTurnPhase())
			{
				// 音なら Surprise_* を維持
				// 視覚なら ↑で LookAround を開始してるのでそれが維持される
			}
			else
			{
				// 待機フェーズは LookAround
				m_pAnimComp->Play(AnimType::LookAround, 0.1f);
			}

			m_PrevAIState = aiState;
			return;
		}

		// それ以外の状態: Idle / Walk / Run を速度に応じて再生
		// それ以外
		Vector3 vel = Vector3::Zero;
		if (m_CharComp) vel = m_CharComp->GetLinearVelocity();
		vel.y = 0.0f;
		const float speed = vel.Length();

		constexpr float MOVE_EPS = 5.0f; // ほぼ停止扱い

		if (aiState == EnemyAIComponent::State::Idle)
		{
			m_pAnimComp->Play(AnimType::Idle, 0.1f);
		}
		else if (aiState == EnemyAIComponent::State::Investigate)
		{
			// 調査中：動いてるなら走り、止まってるなら待機
			m_pAnimComp->Play((speed > MOVE_EPS) ? AnimType::Run : AnimType::Idle, 0.1f);
		}
		else
		{
			// 通常（巡回など）：動いてるなら歩き、止まってるなら待機
			m_pAnimComp->Play((speed > MOVE_EPS) ? AnimType::Walk : AnimType::Idle, 0.1f);
		}
		m_PrevAIState = aiState;
	}
}


void Enemy::Draw(void) const
{
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

void Enemy::SetWayPoints(const Vector3& start, const Vector3& end)
{
	m_StartPos = start;
	m_EndPos = end;

	m_RequestedWayPoints.clear();
	m_RequestedWayPoints.push_back(start);
	m_RequestedWayPoints.push_back(end);

	if (m_AIComp)
	{
		m_AIComp->SetWayPoints(m_RequestedWayPoints);
	}
}

void Enemy::OnFoundPlayer(void)
{
	// すでに演出中なら何もしない
	if (m_GameOverTriggered)
		return;

	m_GameOverTriggered = true;

	// 射撃アニメ再生開始（ここで初期化）
	m_ShotStarted = false;
	m_ShotTimer = 0.0f;

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

			float yaw = std::atan2(-dir.x, -dir.z);   // いま使ってる「プレイヤーへ向く」yaw

			// --- アニメーションの都合で右へ90度回転補正 ---
			constexpr float RIGHT_90 = PI * 0.5f;      // 90度
			yaw += RIGHT_90;                           // もし逆なら yaw -= RIGHT_90 にする

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