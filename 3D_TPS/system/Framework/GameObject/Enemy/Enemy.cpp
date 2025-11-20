#include "Enemy.h"
#include <random>
#include <iostream>
#include "system/Framework/AssetManager/AssetManager.h"
#include "Framework/GameObject/Player/Player.h"
#include "system/Framework/Component/AI/EnemyAIComponent.h"
#include "system/Framework/Component/Physic/CharacterVirtualComponent.h"
#include "system/RandomEngine.h"

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
	// アニメーションオブジェクトを生成
	this->m_pAnimationObject = std::make_unique<CAnimationObject>();
	this->m_pAnimationObject->Init();

	// メッシュを取得
	this->m_pAnimationMesh = AssetManager::GetInstance().GetAnimationMesh("Akai");
	// シェーダーの初期化
	m_Shader.Create("shader/vertexLightingOneSkinVS.hlsl", "shader/vertexLightingPS.hlsl");

	// アニメーションデータ取得
	this->m_pAnimationData = AssetManager::GetInstance().GetAnimationData("Akai_Idle");
	// 現在のアニメーションをセット
	aiAnimation* animation = m_pAnimationData->GetAnimation("Akai_Idle", 0);
	this->m_pCurrentAnimation = animation;
	m_pAnimationMesh->SetCurentAnimation(animation);
	// アニメーションメッシュをセット
	this->m_pAnimationObject->SetAnimationMesh(m_pAnimationMesh);

	// =========================
	//  敵の初期位置をランダム決定
	// =========================
	auto& rng = RandomEngine::tls();           // スレッドローカル RNG

	float mapMinX = -2500.0f;
	float mapMaxX = 2500.0f;
	float mapMinZ = -2500.0f;
	float mapMaxZ = 2500.0f;

	float startX = static_cast<float>(rng.uniformReal(mapMinX, mapMaxX));
	float startZ = static_cast<float>(rng.uniformReal(mapMinZ, mapMaxZ));

	m_StartPos = Vector3(startX, 5.0f, startZ);
	m_Transform.SetPosition(m_StartPos);

	// パトロール範囲
	float patrolRange = 1000.0f;

	// 1つ目の終点（今までの m_EndPos と同じイメージ）
	float endOffsetX = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
	float endOffsetZ = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
	m_EndPos = m_StartPos + Vector3(endOffsetX, 0.0f, endOffsetZ);

	// ==== ここからコンポーネントをアタッチする ====

	m_StartPos = Vector3(500.0f, 0.0f, 0.0f);
	m_EndPos = Vector3(-500.0f, 0.0f, 0.0f);

	// 1) 物理: CharacterVirtualComponent を付ける
	{
		m_CharComp = AddComponent<CharacterVirtualComponent>("EnemyCharacter");
		// 敵用のカプセルサイズ（プレイヤーと同じでも OK）
		m_CharComp->SetCapsule(ENEMY_CAPSULE_HALFHEIGHT, ENEMY_CAPSULE_RADIUS);
		m_CharComp->SetOffset(ENEMY_COLLIDER_OFFSET);
		m_CharComp->Init();
	}

	// 2) AI: EnemyAIComponent を付けて、巡回ルートなどを渡す
	{
		m_AIComp = AddComponent<EnemyAIComponent>("EnemyAI");

		std::vector<Vector3> waypoints;
		waypoints.reserve(4);

		// スタート地点
		waypoints.push_back(m_StartPos);

		// エンド地点（上で作った m_EndPos）
		waypoints.push_back(m_EndPos);

		// 追加のランダムポイント
		/*const int extraCount = 2;
		for (int i = 0; i < extraCount; ++i)
		{
			float ox = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
			float oz = static_cast<float>(rng.uniformReal(-patrolRange, patrolRange));
			Vector3 p = m_StartPos + Vector3(ox, 0.0f, oz);
			waypoints.push_back(p);
		}*/

		// 必要ならメンバにもコピーしておくとデバッグしやすい

		m_AIComp->SetWayPoints(waypoints);
		m_AIComp->SetArriveRadius(50.0f);
		m_AIComp->SetRayLength(300.0f);
		m_AIComp->SetAvoidWeight(1.5f);
		m_AIComp->SetEyeHeight(80.0f);

		m_AIComp->Init();
	}
}

void Enemy::Update(const float deltatime)
{
	if (m_pPlayer && m_pPlayer->IsDestroy()) { m_pPlayer = nullptr; }

	// コンポーネント更新（AI / CharacterVirtual）を呼ぶ
	GameObject::Update(deltatime);

	// ---- ここからはアニメーション状態だけ決める ----

	// 速度ベクトルから「歩きかアイドルか」を決める
	Vector3 vel = Vector3::Zero;
	if (m_CharComp)
	{
		vel = m_CharComp->GetLinearVelocity();
	}

	aiAnimation* animdata = nullptr;

	if (vel.Length() > 0.1f)
	{
		// Run アニメーション
		animdata = AssetManager::GetInstance()
			.GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
	}
	else
	{
		// Idle アニメーション
		animdata = AssetManager::GetInstance()
			.GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
	}

	if (animdata && m_pCurrentAnimation != animdata)
	{
		m_pCurrentAnimation = animdata;
		m_pAnimationMesh->SetCurentAnimation(m_pCurrentAnimation);
	}

	m_pAnimationObject->Update(m_AnimationSpeed);
}


void Enemy::Draw(void) const
{
	// シェーダーをセット
	m_Shader.SetGPU();

	// ワールド行列をセット
	Matrix4x4 worldMatrix = this->GetWorldMatrix();
	Renderer::SetWorldMatrix(&worldMatrix);

	m_pAnimationObject->Draw();
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