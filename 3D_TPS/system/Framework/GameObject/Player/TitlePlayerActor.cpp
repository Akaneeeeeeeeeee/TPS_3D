#include "TitlePlayerActor.h"
#include "system/meshmanager.h"
#include "system/Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "system/Framework/Component/Physic/CharacterVirtualComponent.h"
#include <cmath>

namespace
{
	constexpr float BLEND_SEC = 0.15f;
	constexpr Vector3 UP = Vector3(0, 1, 0);

	// タイトル用の見た目速度（ゲームプレイより演出優先で調整）
	constexpr float TITLE_WALK_SPEED = 120.0f;
	constexpr float TITLE_RUN_SPEED = 220.0f;

	constexpr float AMOUNT_TO_IDLE = 0.05f;
	constexpr float AMOUNT_TO_RUN = 0.90f;

	constexpr float PLAYER_CAPSULE_HALF_HEIGHT = 60.0f; // カプセル高さ(半分)
	constexpr float PLAYER_CAPSULE_RADIUS = 35.0f;      // カプセル半径

	// 足元基準にしたい場合のコライダー起点オフセット（現状未使用なら消してOK）
	constexpr Vector3 PLAYER_COLLIDER_OFFSET = Vector3(0.0f, 80.0f, 0.0f);

	// 姿勢ごとの移動速度係数（現状 CharacterVirtualComponent 側で係数管理しているなら未使用）
	constexpr float CROUCH_MOVE_SPEED_FACTOR = 0.5f;
	constexpr float PRONE_MOVE_SPEED_FACTOR = 0.25f;

}

void TitlePlayerActor::Awake(void)
{
	auto& am = AssetManager::GetInstance();

	// ---- 見た目（スキン） ----
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("TitleSkinnedAnim");

	// メッシュ名はあなたのアセット名に合わせる
	// 例：Playerで使ってた "Akai" を流用
	CAnimationMesh* mesh = am.GetMesh<CAnimationMesh>("Akai");
	m_pAnimComp->SetMesh(mesh);

	CShader* shader = am.GetShader<CShader>("animshader");
	m_pAnimComp->SetShader(shader);

	// ---- クリップ取得 ----
	// あなたの Player の取り方をそのまま使う
	m_Idle = am.GetAnimationData<CAnimationData>("Akai_Idle")->GetAnimation("Akai_Idle", 0);
	m_CrouchWalk = am.GetAnimationData<CAnimationData>("Crouched_Walking")->GetAnimation("Crouched_Walking", 0);
	m_Walk = am.GetAnimationData<CAnimationData>("Walking")->GetAnimation("Walking", 0);
	m_Run = am.GetAnimationData<CAnimationData>("Akai_Run")->GetAnimation("Akai_Run", 0);

	// 初期
	if (m_pAnimComp && m_Idle)
	{
		m_pAnimComp->SetClip(AnimType::Crouch, m_CrouchWalk); // 既存の仕組みがあるならそれを使う
		m_pAnimComp->Play(AnimType::Crouch, BLEND_SEC);
		m_pAnimComp->SetPlaybackSpeed(1.0f);
	}

	// タイトル用は台本で動かすので、MoveSpeedは演出値に
	m_MoveSpeed = TITLE_WALK_SPEED;

	// カメラ追加
	m_pCamera = this->AddComponent<CameraComponent>("TitleCameraComponent");
	m_pCamera->SetPosition(Vector3(500.0f, 100.0f, -750.0f));
	m_pCamera->SetLookAt(Vector3::Zero);

	// タイトル用プレイヤー位置（例）
	Vector3 p = Vector3::Zero;

	// どこを見るか：プレイヤーより少し高い（頭より上）
	Vector3 look = p + Vector3(0.0f, 225.0f, 0.0f);

	// 低い位置から、少し離れて見上げる
	m_pCamera->SetLookAt(look);
	m_pCamera->SetRadius(900.0f);

	// 重要：-110° など「-90°より下」にする（見上げが作れる）
	m_pCamera->SetElevation(DirectX::XMConvertToRadians(-105.0f));

	// 方位角：後ろから撮る → 90° 
	// zがマイナス側に出る
	m_pCamera->SetAzimuth(DirectX::XMConvertToRadians(90.0f));

	// CharacterVirtual 必須（地形当たり判定のため）
	m_pCharaVirtualComp = AddComponent<CharacterVirtualComponent>("TitleCharacterVirtualComponent");
	m_pCharaVirtualComp->SetCapsule(PLAYER_CAPSULE_HALF_HEIGHT, PLAYER_CAPSULE_RADIUS);

	m_MoveDir = Vector3::Right;
	m_MoveAmount = 1.0f;
}

void TitlePlayerActor::Start(void)
{
	// 初期はしゃがみ状態に
	m_IsCrouching = true;

	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetStance(CharacterVirtualComponent::Stance::Crouch);
		m_pCharaVirtualComp->SetMoveInput(Vector3::Zero, 0.0f);
	}
}

void TitlePlayerActor::Update(const float dt)
{
	// 1) 姿勢は「移動してなくても」毎フレ反映（ここ重要）
	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetStance(
			m_IsCrouching ? CharacterVirtualComponent::Stance::Crouch
			: CharacterVirtualComponent::Stance::Stand
		);
	}

	// 2) 台本テレポート（瞬間移動）は Teleport を使う
	if (m_UseTargetPose)
	{
		if (m_pCharaVirtualComp)
		{
			m_pCharaVirtualComp->Teleport(m_TargetPos);
		}

		m_Transform.SetRotation(m_TargetRot);

		if (m_pCharaVirtualComp)
		{
			m_pCharaVirtualComp->SetMoveInput(Vector3::Zero, 0.0f);
		}
	}
	else
	{
		// 3) 台本の dir/amount を CharacterVirtual に渡す（Playerと同じ）
		Vector3 move_dir = m_MoveDir;
		move_dir.y = 0.0f;

		float amount = std::clamp(m_MoveAmount, 0.0f, 1.0f);

		if (amount > AMOUNT_TO_IDLE && move_dir.LengthSquared() > 1e-6f)
		{
			move_dir.Normalize();

			if (m_FaceMoveDir)
			{
				const float yaw = std::atan2(-move_dir.x, -move_dir.z);
				m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(UP, yaw));
			}

			if (m_pCharaVirtualComp)
				m_pCharaVirtualComp->SetMoveInput(move_dir, amount);
		}
		else
		{
			// 止める入力を必ず渡す（慣性・入力残り対策）
			if (m_pCharaVirtualComp)
				m_pCharaVirtualComp->SetMoveInput(Vector3::Zero, 0.0f);
		}
	}

	// 4) ここで CharacterVirtualComponent::Update が走って位置が確定する
	GameObject::Update(dt);

	// 5) アニメは amount / しゃがみ等で決める（必要なら）
	ApplyAnimation(dt);

	// 6) カメラは確定位置を使って更新する（必要なら）
	// UpdateCamera(dt);
}

void TitlePlayerActor::Draw(void) const
{
	if (m_pCamera)
	{
		m_pCamera->ApplyCamera();
	}
	Character::Draw();
}

void TitlePlayerActor::Uninit(void)
{
	Character::Uninit();
}

void TitlePlayerActor::ApplyMovement(float dt)
{
	// 1) 台本が姿勢を直接指定するモード
	if (m_UseTargetPose)
	{
		m_Transform.SetPosition(m_TargetPos);
		m_Transform.SetRotation(m_TargetRot);
		return;
	}

	// 2) dir/amount で動かすモード
	if (m_MoveAmount < AMOUNT_TO_IDLE) return;

	Vector3 dir = m_MoveDir;
	if (dir.LengthSquared() < 1e-6f) return;
	dir.Normalize();

	// amountで walk/run を切替
	const bool isRun = (m_MoveAmount >= AMOUNT_TO_RUN);
	const float speed = isRun ? TITLE_RUN_SPEED : TITLE_WALK_SPEED;

	Vector3 pos = m_Transform.GetPosition();
	pos += dir * (speed * m_MoveAmount * dt);
	m_Transform.SetPosition(pos);

	// 向きを進行方向へ
	if (m_FaceMoveDir)
	{
		// あなたの座標系（前が -Z など）に合わせて符号調整
		const float yaw = std::atan2(-dir.x, -dir.z);
		m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(UP, yaw));
	}
}

void TitlePlayerActor::ApplyAnimation(float deltatime)
{
	if (!m_pAnimComp) return;

	// 動いてないならIdle
	TitleAnim anim = m_TargetAnim;

	// m_TargetAnim を明示してないなら、入力から決めてもいい
	// （台本で SetAnim しない運用の場合）
	if (anim == TitleAnim::Idle)
	{
		if (m_MoveAmount >= AMOUNT_TO_IDLE)
		{
			anim = (m_MoveAmount >= AMOUNT_TO_RUN) ? TitleAnim::Run : TitleAnim::Walk;
		}
	}

	switch (anim)
	{
	case TitleAnim::Idle:
		// 既存のAnimTypeに合わせる
		m_pAnimComp->Play(AnimType::Idle, BLEND_SEC);
		m_pAnimComp->SetPlaybackSpeed(1.0f);
		break;
	case TitleAnim::Walk:
		m_pAnimComp->Play(AnimType::Walk, BLEND_SEC);
		m_pAnimComp->SetPlaybackSpeed(1.0f);
		break;
	case TitleAnim::Run:
		m_pAnimComp->Play(AnimType::Run, BLEND_SEC);
		m_pAnimComp->SetPlaybackSpeed(1.0f);
		break;
	}
}
