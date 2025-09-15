#include "Enemy.h"
#include <random>
#include <iostream>

Enemy::Enemy(uint64_t id, const std::string& name, const Tag& tag)
	: Character(id, name, tag)
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

	// ランダム生成器
	static std::random_device rd;
	static std::mt19937 mt(rd());

	// マップ内のランダム開始位置範囲
	float mapMinX = -50.0f;
	float mapMaxX = 500.0f;
	float mapMinZ = -50.0f;
	float mapMaxZ = 500.0f;
	std::uniform_real_distribution<float> distX(mapMinX, mapMaxX);
	std::uniform_real_distribution<float> distZ(mapMinZ, mapMaxZ);

	// 開始位置をランダムに設定
	m_StartPos = Vector3(distX(mt), 0.0f, distZ(mt));
	m_Transform.SetPosition(m_StartPos);

	// 終点は開始位置から一定範囲内
	float patrolRange = 1000.0f; // +-1000の範囲で終点を決める
	std::uniform_real_distribution<float> offsetDist(-patrolRange, patrolRange);
	m_EndPos = m_StartPos + Vector3(offsetDist(mt), 0.0f, offsetDist(mt));

	m_TargetPos = m_EndPos;
}

void Enemy::Update(uint64_t deltatime)
{
	// アニメーション比較用変数	this->m_Transform.SetPosition(Vector3(500.0f, 0.0f, 500.0f));

	aiAnimation* animdata;
	
	// 目標地点同士を往復する
	Vector3 pos = m_Transform.GetPosition();
	std::cout << "Enemy Pos: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
	Vector3 dir = m_TargetPos - pos;
	
	// 移動ベクトルが0でなければ正規化して移動
	if (dir.Length() > 5.0f) {
		// 移動ベクトルを正規化
		dir.Normalize();
		pos += dir * m_MoveSpeed;
		m_Transform.SetPosition(pos);

		// 向きも更新
		float targetYaw = std::atan2(-dir.x, -dir.z);
		Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), targetYaw);
		m_Transform.SetRotation(q);

		// 移動していれば移動アニメーションを再生
		// アニメーションデータ取得
		// アニメーションを Run に切り替え
		animdata = AssetManager::GetInstance().GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
		if (m_pCurrentAnimation != animdata) {
			m_pCurrentAnimation = animdata;
			m_pAnimationMesh->SetCurentAnimation(m_pCurrentAnimation);
		}
	}
	else {
		// 目標到達 → 反転
		if (m_GoingToEnd) {
			m_TargetPos = m_StartPos;
			m_GoingToEnd = false;
		}
		else {
			m_TargetPos = m_EndPos;
			m_GoingToEnd = true;
		}

		// 移動してない場合はアニメーションをアイドル状態に変更
		// アニメーションを Idle に切り替え
		animdata = AssetManager::GetInstance().GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
		if (m_pCurrentAnimation != animdata) {
			m_pCurrentAnimation = animdata;
			m_pAnimationMesh->SetCurentAnimation(m_pCurrentAnimation);
		}
	}

	// アニメーションの更新
	m_pAnimationObject->Update(m_AnimationSpeed);
}

void Enemy::Draw(uint64_t deltatime)
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