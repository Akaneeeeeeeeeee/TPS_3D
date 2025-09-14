#include "Enemy.h"


Enemy::Enemy(uint64_t id, const std::string& name, const Tag& tag)
	: Character(id, name, tag)
{
}

Enemy::~Enemy()
{
}


void Enemy::Init(void)
{
	Character::Init();
}

void Enemy::Update(uint64_t deltatime)
{

	Vector3 move(0, 0, 0);
	Vector3 pos = m_Transform.GetPosition();

	//if (input.CheckKeyBuffer(DIK_W)) { move.z += 1.0f; }
	//if (input.CheckKeyBuffer(DIK_S)) { move.z -= 1.0f; }
	//if (input.CheckKeyBuffer(DIK_A)) { move.x -= 1.0f; }
	//if (input.CheckKeyBuffer(DIK_D)) { move.x += 1.0f; }

	aiAnimation* animdata;

	// 移動ベクトルが0でなければ正規化して移動
	if (move.LengthSquared() > 0.0f) {
		// 移動ベクトルを正規化
		move.Normalize();
		// 入力に基づいてキャラクターの向きを更新
		float targetYaw = std::atan2(-move.x, -move.z);

		// Y軸回転をクォータニオンで適用
		Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), targetYaw);

		// 前方向ベクトルを計算して移動
		Matrix4x4 rotY = Matrix4x4::CreateFromQuaternion(q);
		Vector3 forward = Vector3::TransformNormal(Vector3(0, 0, -1), rotY);

		// 移動
		pos += forward * m_MoveSpeed;

		// Transform更新
		m_Transform.SetPosition(pos);
		m_Transform.SetRotation(q);

		// 入力があれば移動アニメーションを再生
		// アニメーションデータ取得
		// アニメーションを Run に切り替え
		animdata = AssetManager::GetInstance().GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
		if (m_pCurrentAnimation != animdata) {
			m_pCurrentAnimation = animdata;
			m_pAnimationMesh->SetCurentAnimation(m_pCurrentAnimation);
		}
	}
	else {
		// 入力がない場合はアニメーションをアイドル状態に変更
		// アニメーションデータ取得
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
	Character::Draw(deltatime);
}

void Enemy::Uninit(void)
{
	Character::Uninit();
}