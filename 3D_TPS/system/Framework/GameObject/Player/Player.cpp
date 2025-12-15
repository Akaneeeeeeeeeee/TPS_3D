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
#include "system/Framework/Component/Camera/CameraComponent.h"

namespace {
	constexpr float PLAYER_CAPSULE_HALFHEIGHT = 60.0f;	// プレイヤーカプセルコライダーの高さ(半分)
	constexpr float PLAYER_CAPSULE_RADIUS = 35.0f;		// プレイヤーカプセルコライダーの半径
	constexpr Vector3 PLAYER_COLLIDER_OFFSET = Vector3(0.0f, 80.0f, 0.0f);
	constexpr float CROUCH_MOVESPEED_FACTOR = 0.5f;     // しゃがみ移動速度係数
	constexpr float PRONE_MOVESPEED_FACTOR = 0.25f;     // 伏せ移動速度係数


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
	//CAnimationMesh* mesh = am.GetAnimationMesh("Akai");
	CAnimationMesh* mesh = am.GetMesh<CAnimationMesh>("Akai");

	m_pAnimComp->SetMesh(mesh);
	//CShader* shader = MeshManager::getShader<CShader>("animshader");
	CShader* shader = am.GetShader<CShader>("animshader");
	m_pAnimComp->SetShader(shader);

	// 3) アニメーションデータを全部キャッシュ
	auto* idle = am.GetAnimationData<CAnimationData>("Akai_Idle")->GetAnimation("Akai_Idle", 0);
	auto* walk = am.GetAnimationData<CAnimationData>("Walking")->GetAnimation("Walking", 0);
	auto* run = am.GetAnimationData<CAnimationData>("Akai_Run")->GetAnimation("Akai_Run", 0);
	auto* crouch = am.GetAnimationData<CAnimationData>("Crouching_Idle")->GetAnimation("Crouching_Idle", 0);
	auto* crouchwalk = am.GetAnimationData<CAnimationData>("Crouched_Walking")->GetAnimation("Crouched_Walking", 0);
	//auto* idle = am.GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
	//auto* walk = am.GetAnimationData("Walking")->GetAnimation("Walking", 0);
	//auto* run = am.GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
	//auto* crouch = am.GetAnimationData("Crouching_Idle")->GetAnimation("Crouching_Idle", 0);
	//auto* crouchwalk = am.GetAnimationData("Crouched_Walking")->GetAnimation("Crouched_Walking", 0);

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
	}
	// TPS カメラコンポーネント追加
	{
		m_pCamera = this->AddComponent<CameraComponent>(m_Name + "_CameraComponent");

		// 必要なら初期パラメータを調整
		m_pCamera->SetRadius(800.0f);
		// m_pCamera->SetElevation();
		// m_pCamera->SetAzimuth();
	}
}

// プレイヤー更新
void Player::Update(const float deltatime)
{
	auto& input = CDirectInput::GetInstance();

	Vector3 input_dir = Vector3::Zero;

	// ---- 1) 入力から移動方向を作る ----
	if (input.CheckKeyBuffer(DIK_W)) { input_dir.z += 1.0f; }
	if (input.CheckKeyBuffer(DIK_S)) { input_dir.z -= 1.0f; }
	if (input.CheckKeyBuffer(DIK_A)) { input_dir.x -= 1.0f; }
	if (input.CheckKeyBuffer(DIK_D)) { input_dir.x += 1.0f; }

	// パッド
	Vector2 stick = input.GetLeftStick();
	input_dir.x += stick.x;
	input_dir.z += stick.y;

	// 倒し具合（0..1） ※キーの斜め(sqrt2)もここで1に丸める
	const float len = std::sqrt(input_dir.x * input_dir.x + input_dir.z * input_dir.z);
	const float amount = std::clamp(len, 0.0f, 1.0f);


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
	if (amount > 1e-4f)
	{
		// 方向だけ正規化
		move_dir = Vector3(input_dir.x / len, 0.0f, input_dir.z / len);

		// 向き（回転だけ）
		float targetYaw = std::atan2(-move_dir.x, -move_dir.z);
		m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), targetYaw));
	}

	// --- 2) 姿勢（しゃがみ） ---
	const bool isCrouching = input.CheckKeyBuffer(DIK_C);
	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetStance(
			isCrouching ? CharacterVirtualComponent::Stance::Crouch
			: CharacterVirtualComponent::Stance::Stand
		);
	}

	// --- 3) CharacterVirtual に「方向 + 量」を渡す ---
	if (m_pCharaVirtualComp)
	{
		// しゃがみ時は移動速度を落とす
		m_pCharaVirtualComp->SetMoveInput(move_dir, amount);
		if (input.CheckKeyBuffer(DIK_SPACE))
			m_pCharaVirtualComp->RequestJump();
	}

	// --- 4) アニメ（量に応じて速度を変える） ---
	if (m_pAnimComp)
	{
		// 立ちの場合: amount 小→歩き、大→走り にしたいなら閾値で切替
		// 連続にしたいなら同じクリップで speed を変えるほうが実装が簡単
		if (amount < 0.05f)
		{
			m_pAnimComp->Play(isCrouching ? AnimType::Crouch : AnimType::Idle, 0.1f);
			m_pAnimComp->SetPlaybackSpeed(1.0f);  // ★後述の追加API
		}
		else
		{
			if (isCrouching)
			{
				m_pAnimComp->Play(AnimType::CrouchWalk, 0.1f);

				// しゃがみは控えめに
				float speed = std::lerp(0.6f, 1.1f, amount);
				m_pAnimComp->SetPlaybackSpeed(speed);
			}
			else
			{
				// 「歩きクリップを使う」場合は amount に応じて切り替え
				// 例: amount < 0.6 なら Walk、それ以上は Run
				if (amount < 0.6f) m_pAnimComp->Play(AnimType::Walk, 0.1f);
				else               m_pAnimComp->Play(AnimType::Run, 0.1f);

				float speed = std::lerp(0.8f, 1.5f, amount);
				m_pAnimComp->SetPlaybackSpeed(speed);
			}
		}
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
			// 姿勢ごとの係数を取得
			float kInterval = m_pCharaVirtualComp->GetFootstepIntervalCoeff();
			float kRadius = m_pCharaVirtualComp->GetFootstepRadiusCoeff();
			float kLoudness = m_pCharaVirtualComp->GetFootstepLoudnessCoeff();

			// 実際の足音間隔 = 立ち基準 * 姿勢係数
			float interval = FOOTSTEP_BASE_INTERVAL * kInterval;

			m_FootstepTimer += deltatime;

			if (m_FootstepTimer >= interval)
			{
				m_FootstepTimer = 0.0f;

				// 足音の WorldSoundEvent を飛ばす
				WorldSoundEvent ev{};
				ev.Position = GetPosition();
				ev.Radius = FOOTSTEP_BASE_RADIUS * kRadius;
				ev.Loudness = FOOTSTEP_BASE_LOUDNESS * kLoudness;
				ev.Volume = 1.0f;
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

			// そこそこ高いところから落ちたときだけ
			if (impact > 200.0f)
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

		// TPS なのでカメラはプレイヤーから一定距離離れる
		Vector3 lookAt = pos;
		lookAt.y += 100.0f;  // 注視点を少し上にずらす
		m_pCamera->SetLookAt(lookAt);
	}
}


void Player::Draw(void) const
{
	if (m_pCamera)
	{
		// カメラ行列計算＋Renderer へのセット
		m_pCamera->ApplyCamera();
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
	float topY = 2.0f * (hh + r);	// カプセルの上端近辺
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
	Vector3 side = viewDir.Cross(up);
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