#include "Player.h"
#include "system/CDirectInput.h"
#include "Framework/Component/Physic/CapsuleCollider.h"
#include "Framework/Component/Physic/BoxCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "Framework/Component/Physic/CharacterVirtualComponent.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "Framework/Time/Time.h"
#include "system/Sound/WorldSoundEvent.h"
#include "system/Framework/SoundManager/SoundManager.h"
#include "system/meshmanager.h"

namespace {
	constexpr float PLAYER_CAPSULE_HALFHEIGHT = 60.0f;
	constexpr float PLAYER_CAPSULE_RADIUS = 35.0f;
	constexpr Vector3 PLAYER_COLLIDER_OFFSET = Vector3(0.0f, 80.0f, 0.0f);
}

Player::Player(ComponentFactory* factory, const uint64_t id,
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: Character(factory, id, name, tag, transform)
{
}

Player::~Player()
{
}

void Player::Init(void)
{
	auto& am = AssetManager::GetInstance();

	// 1) コンポーネント追加
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("SkinnedAnim");

	// 2) メッシュとシェーダ設定
	CAnimationMesh* mesh = am.GetAnimationMesh("Akai");
	m_pAnimComp->SetMesh(mesh);
	CShader* shader = MeshManager::getShader<CShader>("animshader");
	m_pAnimComp->SetShader(shader);

	// 3) アニメーションデータを全部キャッシュ
	auto* idle = am.GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
	auto* walk = am.GetAnimationData("Walking")->GetAnimation("Walking", 0);
	auto* run = am.GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
	auto* crouch = am.GetAnimationData("Crouching_Idle")->GetAnimation("Crouching_Idle", 0);
	auto* crouchwalk = am.GetAnimationData("Crouched_Walking")->GetAnimation("Crouched_Walking", 0);

	m_pAnimComp->SetClip(AnimType::Idle, idle);
	m_pAnimComp->SetClip(AnimType::Crouch, crouch);
	m_pAnimComp->SetClip(AnimType::CrouchWalk, crouchwalk);
	m_pAnimComp->SetClip(AnimType::Walk, walk);
	m_pAnimComp->SetClip(AnimType::Run, run);

	// ステータスを設定
	this->m_MoveSpeed = 10.0f;
	this->m_AnimationSpeed = 1.0f;


	// 移動制御用コンポーネントを追加
	{
		m_pCharaVirtualComp = this->AddComponent<CharacterVirtualComponent>(m_Name + "_CharacterVirtualComponent");
		m_pCharaVirtualComp->SetCapsule(PLAYER_CAPSULE_HALFHEIGHT, PLAYER_CAPSULE_RADIUS);
		m_pCharaVirtualComp->SetOffset(PLAYER_COLLIDER_OFFSET);
	}
}

// プレイヤー更新
void Player::Update(const float deltatime)
{
	CDirectInput& input = CDirectInput::GetInstance();

	// ---- 1) 入力から移動方向を作る ----
	Vector3 input_dir(0, 0, 0);
	if (input.CheckKeyBuffer(DIK_W)) { input_dir.z += 1.0f; }
	if (input.CheckKeyBuffer(DIK_S)) { input_dir.z -= 1.0f; }
	if (input.CheckKeyBuffer(DIK_A)) { input_dir.x -= 1.0f; }
	if (input.CheckKeyBuffer(DIK_D)) { input_dir.x += 1.0f; }

	// しゃがみキー（Cキー）押下中かどうか
	bool isCrouching = input.CheckKeyBuffer(DIK_C);

#ifdef _DEBUG
	if (input.CheckKeyBuffer(DIK_K)) {
		Time::GetInstance().SetTimeScale(0.1f);
	}
	// E キーが押された瞬間に音を出す
	if (input.CheckKeyBuffer(DIK_E))
	{
		WorldSoundEvent ev{};
		ev.Position = GetPosition();
		ev.Radius = 1000.0f;
		ev.Loudness = 1.0f;
		ev.Type = SoundType::Footstep; // とりあえず足音扱い

		SoundManager::GetInstance().EmitSound(ev);
	}
#endif // _DEBUG

	bool wants_jump = input.CheckKeyBuffer(DIK_SPACE);

	Vector3 move_dir = Vector3::Zero;

	// ----- アニメーションと移動方向の決定 -----
	if (input_dir.LengthSquared() > 0.0f)
	{
		move_dir = input_dir;
		move_dir.Normalize();

		// ---- 2) 進行方向を向く（回転だけ）----
		float targetYaw = std::atan2(-move_dir.x, -move_dir.z);
		// Y軸回転をクォータニオンで適用
		Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), targetYaw);
		m_Transform.SetRotation(q);
	}

	// ---- どのアニメを再生したいかだけ決める ----
	float mag = input_dir.Length();
	if (m_pAnimComp)
	{
		if (isCrouching)
		{
			if (mag < 0.1f)
			{
				// しゃがみ＋入力なし → しゃがみ待機
				m_pAnimComp->Play(AnimType::Crouch, 0.1f);
			}
			else
			{
				// しゃがみ＋入力あり → しゃがみ歩き
				m_pAnimComp->Play(AnimType::CrouchWalk, 0.1f);
			}
		}
		else
		{
			if (mag < 0.1f)
			{
				// 立ち＋入力なし → 通常待機
				m_pAnimComp->Play(AnimType::Idle, 0.1f);
			}
			else
			{
				// 立ち＋入力あり → 走り
				m_pAnimComp->Play(AnimType::Run, 0.1f);
			}
		}
	}

	// ---- 3) CharacterVirtual に入力を渡す ----
	if (m_pCharaVirtualComp)
	{
		// 方向だけ渡す（速さは CharVirtual 側の m_MoveAccel で調整）
		m_pCharaVirtualComp->SetMoveDir(move_dir);

		if (wants_jump)
			m_pCharaVirtualComp->RequestJump();
	}

	// ---- 4) コンポーネント更新（ここで位置が決まる）----
	GameObject::Update(deltatime);

	// ---- 5) 位置を使ってカメラ更新 ----
	Vector3 pos = m_Transform.GetPosition();

	if (m_pCamera)
	{
		static float azimuth = m_pCamera->GetAzimuth();
		static float elevation = m_pCamera->GetElevation();

		if (input.GetMouseRButtonCheck())
		{
			LONG dx = input.GetMouseStateData().lX;
			LONG dy = input.GetMouseStateData().lY;

			// マウス感度
			float sensitivity = 0.005f;
			// マウスの移動量に応じてカメラの角度を更新
			azimuth += dx * sensitivity;
			elevation -= dy * sensitivity;

			// 仰角の制限(-89°～89°の範囲に制限)
			const float limit = (PI / 2.0f) - 0.01f;
			if (elevation > limit) elevation = limit;
			if (elevation < -limit) elevation = -limit;
		}

		m_pCamera->SetRadius(800.0f);
		m_pCamera->SetAzimuth(azimuth);
		m_pCamera->SetElevation(elevation);

		// カメラ位置更新
		// TPSなのでカメラはプレイヤーから一定距離離れる
		Vector3 lookAt = pos;
		lookAt.y += 100.0f;			// 注視点を少し上にずらす
		m_pCamera->SetLookat(lookAt);
		m_pCamera->CalcCameraPositionTranslate(lookAt);
	}
}


void Player::Draw(void) const
{
	if (m_pCamera)
	{
		this->m_pCamera->Draw();
	}

	Character::Draw();
}

void Player::Uninit(void)
{
	GameObject::Uninit();
}