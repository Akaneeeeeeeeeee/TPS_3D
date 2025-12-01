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
	//CAnimationMesh* mesh = am.GetAnimationMesh("maincharacter");
	//CAnimationMesh* mesh = am.GetAnimationMesh("character");
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

	// ---- 3) アニメーション選択 ----
	float mag = input_dir.Length();
	if (m_pAnimComp)
	{
		if (isCrouching)
		{
			// しゃがみ姿勢に設定
			m_pCharaVirtualComp->SetStance(CharacterVirtualComponent::Stance::Crouch);

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
			// 立ち姿勢に設定
			m_pCharaVirtualComp->SetStance(CharacterVirtualComponent::Stance::Stand);

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

	// ---- 4) CharacterVirtual に入力を渡す ----
	if (m_pCharaVirtualComp)
	{
		// 方向だけ渡す（速さは CharVirtual 側の m_MoveAccel で調整）
		m_pCharaVirtualComp->SetMoveDir(move_dir);

		if (wants_jump)
			m_pCharaVirtualComp->RequestJump();
	}

	// ---- 5) コンポーネント更新（ここで位置が決まる）----
	GameObject::Update(deltatime);

	// ---- 6) CharacterVirtual の接地判定に同期して足音を出す ----
	if (m_pCharaVirtualComp && m_FootstepEnabled)
	{
		bool  onGround = m_pCharaVirtualComp->IsOnGround();
		float horizontalSpeed = m_pCharaVirtualComp->GetHorizontalSpeed();

		// 速度がごく小さい場合は「足音なし」とみなす
		const float moveThreshold = 5.0f; // ユニット/秒（調整用）

		bool isMoving = horizontalSpeed > moveThreshold;

		if (onGround && isMoving)
		{
			// しゃがみ中かどうかで間隔を変える
			float interval = isCrouching ? m_FootstepIntervalCrouch : m_FootstepIntervalRun;

			m_FootstepTimer += deltatime;

			if (m_FootstepTimer >= interval)
			{
				m_FootstepTimer = 0.0f;

				// 足音の WorldSoundEvent を飛ばす
				WorldSoundEvent ev{};
				ev.Position = GetPosition();
				ev.Radius = 800.0f;   // 聞こえる距離（調整用）
				ev.Loudness = isCrouching ? 0.4f : 1.0f; // しゃがみは小さく
				ev.Volume = 1.0f;     // 実音量（オーディオ側用に使いたければ）
				ev.Type = SoundType::Footstep;

				SoundManager::GetInstance().EmitSound(ev);
			}
		}
		else
		{
			// 空中や停止中はタイマーをリセット
			m_FootstepTimer = 0.0f;
		}

		// 着地音をつけたいならここで「前フレーム非接地 → 今フレーム接地」を見る
		if (!m_WasOnGround && onGround)
		{
			// 着地時の縦速度から強さを決めるなども可能
			Vector3 v = m_pCharaVirtualComp->GetLinearVelocity();
			float   vy = v.y;
			float   impact = std::max(0.0f, -vy); // 下向き速度

			if (impact > 200.0f) // そこそこ高いところから落ちたときだけ
			{
				WorldSoundEvent ev{};
				ev.Position = GetPosition();
				ev.Radius = 900.0f;
				ev.Loudness = std::clamp(impact / 500.0f, 0.5f, 2.0f); // 雑にスケール
				ev.Type = SoundType::Footstep; // 着地専用種別を作ってもいい

				SoundManager::GetInstance().EmitSound(ev);
			}
		}

		m_WasOnGround = onGround;
	}

	// ---- 7) 位置を使ってカメラ更新 ----
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

void Player::GetVisibilitySamplePoints(const Vector3& eyePos, std::vector<Vector3>& out) const
{
	out.clear();

	auto* ch = m_pCharaVirtualComp;
	if (!ch)
	{
		// 最低限、頭くらいの 1 点だけでも返す
		out.push_back(GetPosition() + Vector3(0.0f, 80.0f, 0.0f));
		return;
	}

	float hh = ch->GetCurrentHalfHeight();
	float r = ch->GetRadius();

	Vector3 foot = GetPosition();   // 足元（カプセルの下端近辺）
	float topY = 2.0f * (hh + r); // カプセルの上端近辺
	float midY = 0.5f * topY;

	Vector3 centerMid = foot + Vector3(0.0f, midY, 0.0f);
	Vector3 top = foot + Vector3(0.0f, topY - r, 0.0f); // 頭寄り
	Vector3 bottom = foot + Vector3(0.0f, r, 0.0f); // 足寄り

	// 敵から見た左右方向を決める
	Vector3 viewDir = centerMid - eyePos;
	if (viewDir.LengthSquared() < 1e-6f)
	{
		viewDir = Vector3(0.0f, 0.0f, -1.0f);
	}
	viewDir.Normalize();

	Vector3 up(0.0f, 1.0f, 0.0f);
	Vector3 side = viewDir.Cross(up); // SimpleMath ならこう
	if (side.LengthSquared() < 1e-6f)
	{
		side = Vector3(1.0f, 0.0f, 0.0f);
	}
	side.Normalize();

	// 中心線
	out.push_back(centerMid);
	out.push_back(top);
	out.push_back(bottom);

	// 左右（楕円の横方向）
	out.push_back(centerMid + side * r);
	out.push_back(centerMid - side * r);
}

JPH::BodyID Player::GetInnerBodyID(void) const
{
	if (m_pCharaVirtualComp)
	{
		return m_pCharaVirtualComp->GetInnerBodyID();
	}
	return JPH::BodyID();
}