#include "Player.h"
#include "system/CDirectInput.h"
#include "Framework/Component/Physic/CharacterVirtualComponent.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "system/Sound/WorldSoundEvent.h"
#include "system/Framework/Component/Camera/CameraComponent.h"
#include "Framework/Component/Throw/ThrowComponent.h"
#include "Framework/Component/Renderer/MeshRenderer/SkinnedMeshRendererComponent.h"
#include "Framework/Component/Sound/SoundEmitterComponent.h"
#include "Framework/Component/Sound/ThrowAudioComponent.h"

#include <algorithm> // std::clamp
#include <cmath>     // std::sqrt, std::atan2, std::lerp

namespace
{
	//============================
	// Player: 形状 / 初期設定
	//============================

	// プレイヤーカプセル
	constexpr float PLAYER_CAPSULE_HALF_HEIGHT = 60.0f; // カプセル高さ(半分)
	constexpr float PLAYER_CAPSULE_RADIUS = 35.0f;      // カプセル半径

	// 足元基準にしたい場合のコライダー起点オフセット（現状未使用なら消してOK）
	constexpr Vector3 PLAYER_COLLIDER_OFFSET = Vector3(0.0f, 80.0f, 0.0f);

	// 姿勢ごとの移動速度係数（現状 CharacterVirtualComponent 側で係数管理しているなら未使用）
	constexpr float CROUCH_MOVE_SPEED_FACTOR = 0.5f;
	constexpr float PRONE_MOVE_SPEED_FACTOR = 0.25f;

	// よく使うベクトル
	constexpr Vector3 WORLD_UP = Vector3(0.0f, 1.0f, 0.0f);

	//============================
	// Player: 入力 → 移動量判定
	//============================

	// amount（入力の倒し具合）がこれ未満なら「入力なし」と同等扱い
	constexpr float MOVE_AMOUNT_EPSILON = 1e-4f;

	// Idle / CrouchIdle に戻す閾値（0..1）
	constexpr float MOVE_AMOUNT_TO_IDLE = 0.05f;

	// Walk / Run の切替閾値（0..1）
	constexpr float MOVE_AMOUNT_WALK_TO_RUN = 0.9f;

	//============================
	// Player: アニメ遷移 / 再生速度
	//============================

	// クリップ切替時の補間時間（秒）
	constexpr float ANIM_BLEND_TIME_SEC = 0.15f;

	// 待機時の再生速度
	constexpr float ANIM_SPEED_IDLE = 1.0f;

	// しゃがみ歩き：スティック量(0..1) → 再生速度
	constexpr float ANIM_SPEED_CROUCH_WALK_MIN = 0.60f;
	constexpr float ANIM_SPEED_CROUCH_WALK_MAX = 1.0f;

	// 歩き：スティック量(0..1) → 再生速度
	constexpr float ANIM_SPEED_WALK_MIN = 0.80f;
	constexpr float ANIM_SPEED_WALK_MAX = 1.0f;

	// 走り：スティック量(0..1) → 再生速度
	constexpr float ANIM_SPEED_RUN_MIN = 0.90f;
	constexpr float ANIM_SPEED_RUN_MAX = 1.0f;

	//============================
	// Player: 足音 / 着地音
	//============================

	// 水平速度がこれ未満なら「移動していない」とみなす（ユニット/秒）
	constexpr float FOOTSTEP_MOVE_SPEED_THRESHOLD = 5.0f;

	// 着地音を出す縦速度しきい値（下向き速度がこれ以上）
	constexpr float LANDING_IMPACT_THRESHOLD = 200.0f;

	// 着地音の届く半径（ユニット）
	constexpr float LANDING_SOUND_RADIUS = 900.0f;

	// 着地音の音量スケール（impact / これ）
	constexpr float LANDING_LOUDNESS_DIV = 500.0f;

	// 着地音の音量クランプ
	constexpr float LANDING_LOUDNESS_MIN = 0.50f;
	constexpr float LANDING_LOUDNESS_MAX = 2.00f;

	// ---- 速度に応じて足音の「強さ/届く範囲」を変えるためのパラメータ ----
	// horizontalSpeed が FOOTSTEP_SPEED_MIN のとき最小、FOOTSTEP_SPEED_MAX で最大になる。
	constexpr float FOOTSTEP_SPEED_MIN = 60.0f;   // 歩き時の水平速度の目安
	constexpr float FOOTSTEP_SPEED_MAX = 250.0f;  // 走り時の水平速度の目安

	// 速度が遅い時/速い時の Loudness 係数（AIに聞こえる強さ & 実再生音量に使う）
	constexpr float FOOTSTEP_SPEED_LOUDNESS_SCALE_MIN = 0.60f;
	constexpr float FOOTSTEP_SPEED_LOUDNESS_SCALE_MAX = 1.60f;

	// 速度で半径も変えたい場合
	constexpr float FOOTSTEP_SPEED_RADIUS_SCALE_MIN = 0.80f;
	constexpr float FOOTSTEP_SPEED_RADIUS_SCALE_MAX = 1.30f;

	// 0..1 に正規化（範囲外はクランプ）
	inline float REMAP_01_CLAMP(float v, float inMin, float inMax)
	{
		const float denom = (inMax - inMin);
		if (denom <= 1e-6f) return 0.0f;
		const float t = (v - inMin) / denom;
		return std::clamp(t, 0.0f, 1.0f);
	}

	//============================
	// Player: TPS カメラ
	//============================

	// TPSカメラの距離（ユニット）
	constexpr float CAMERA_RADIUS = 800.0f;

	// 注視点をプレイヤー位置からどれだけ上にずらすか（ユニット）
	constexpr float CAMERA_LOOK_AT_HEIGHT = 100.0f;

	// マウス感度（ラジアン/マウス移動量）
	constexpr float CAMERA_MOUSE_SENSITIVITY = 0.005f;

	// 仰角制限の余白（ラジアン）
	constexpr float CAMERA_ELEVATION_LIMIT_EPSILON_RAD = 0.01f;

	//============================
	// 可視判定サンプル点（Visibility）
	//============================

	// CharacterVirtual が無い場合に返す「頭位置の高さ」
	constexpr float VISIBILITY_FALLBACK_HEAD_HEIGHT = 80.0f;

	// ベクトルがほぼゼロか判定するためのしきい値（LengthSquared で比較）
	constexpr float VISIBILITY_EPSILON_SQ = 1e-6f;

	// 視線方向が作れないときの既定方向
	constexpr Vector3 VISIBILITY_DEFAULT_VIEW_DIR = Vector3(0.0f, 0.0f, -1.0f);

	// side ベクトルが作れないときの既定方向
	constexpr Vector3 VISIBILITY_DEFAULT_SIDE_DIR = Vector3(1.0f, 0.0f, 0.0f);

	// 構え判定（押し込みとみなす閾値）
	constexpr float AIM_TRIGGER_THRESHOLD = 0.75f;

	// 投げ判定（右トリガーの押し込み）
	constexpr float THROW_TRIGGER_THRESHOLD = 0.25f;

	// 構え中カメラ（調整用）
	constexpr float CAMERA_RADIUS_AIM = 125.0f;
	constexpr float CAMERA_SHOULDER_AIM = 60.0f;     // +右肩 / -左肩
	constexpr float CAMERA_LOOK_AT_HEIGHT_AIM = 175.0f;
	constexpr float CAMERA_NEAR_AIM = 60.0f;         // 足/体を切りやすい

	// 通常時
	constexpr float CAMERA_RADIUS_NORMAL = CAMERA_RADIUS;
	constexpr float CAMERA_SHOULDER_NORMAL = 0.0f;
	constexpr float CAMERA_LOOK_AT_HEIGHT_NORMAL = CAMERA_LOOK_AT_HEIGHT;
	constexpr float CAMERA_NEAR_NORMAL = 1.0f;

	// カメラ補間速度（大きいほど早い）
	constexpr float CAMERA_SMOOTH_SPEED = 12.0f;

	// 構え中に「下を向ける最低角」（足が映りにくい）
	constexpr float AIM_ELEVATION_MIN_DOWN = -0.15f; // 調整

	// 構え中は移動を止めるか（足アニメが無いなら止めるのが破綻しない）
	constexpr bool AIM_LOCK_MOVE = true;

	inline float SmoothTo(float cur, float target, float dt, float speed)
	{
		if (dt <= 0.0f) return target;
		const float t = 1.0f - std::exp(-speed * dt);
		return std::lerp(cur, target, t);
	}

	// 構え時：カメラの注視点を「前方へどれだけ伸ばすか」
	// ここを伸ばすほど「プレイヤーの向いている方向を向く」感じが強くなる
	constexpr float AIM_LOOK_DISTANCE = 2500.0f;

	// リセンター時の仰角（見下ろしにならない安定値）
	constexpr float CAMERA_ELEVATION_RECENTER = -0.25f; // 調整

	constexpr float FOOTSTEP_WALK_INTERVAL = 0.45f;
	constexpr float FOOTSTEP_RUN_INTERVAL = 0.25f; // 走りは頻繁（短い）
	constexpr float FOOTSTEP_VOL_MIN = 0.25f;
	constexpr float FOOTSTEP_VOL_MAX = 1.00f;
	constexpr float FOOTSTEP_RADIUS_MIN = 350.0f;
	constexpr float FOOTSTEP_RADIUS_MAX = 900.0f;

	static Vector3 SmoothToVec3(const Vector3& cur, const Vector3& target, float dt, float speed)
	{
		if (dt <= 0.0f) return target;
		const float t = 1.0f - std::exp(-speed * dt);
		return cur + (target - cur) * t;
	}
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

void Player::Awake(void)
{
	// 1) コンポーネント追加
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("SkinnedAnim");

	SkinnedAnimSetup setup{};
	setup.meshName = "Akai";
	setup.shaderName = "animshader";
	setup.clips = {
		{ AnimType::Idle,       "Akai_Idle",         "Akai_Idle",         0, 1.0f },
		{ AnimType::Walk,       "Walking",          "Walking",          0, 1.0f },
		{ AnimType::Run,        "Akai_Run",         "Akai_Run",         0, 1.0f },
		{ AnimType::Crouch,     "Crouching_Idle",   "Crouching_Idle",   0, 1.0f },
		{ AnimType::CrouchWalk, "Crouched_Walking", "Crouched_Walking", 0, 1.0f },
		{ AnimType::StoneThrow, "StoneThrow",       "StoneThrow",       0, 1.0f },
	};

	m_pAnimComp->SetupFromAssets(setup);

	// ステータスを設定
	this->m_MoveSpeed = 10.0f;
	this->m_AnimationSpeed = 1.0f;

	// 移動制御用コンポーネントを追加
	{
		m_pCharVirtual = this->AddComponent<CharacterVirtualComponent>(m_Name + "_CharacterVirtualComponent");
		m_pCharVirtual->SetCapsule(PLAYER_CAPSULE_HALF_HEIGHT, PLAYER_CAPSULE_RADIUS);
	}

	// TPS カメラコンポーネント追加
	{
		m_pCamera = this->AddComponent<CameraComponent>(m_Name + "_CameraComponent");
		m_pCamera->SetRadius(CAMERA_RADIUS);
	}

	// ThrowComponent（投げ共通）
	{
		m_pThrowComp = this->AddComponent<ThrowComponent>("ThrowComponent");
	}

	// 描画（RenderManagerに出す）
	auto* r = AddComponent<SkinnedMeshRendererComponent>("SkinnedRenderer");
	r->SetMeshKey("Akai");
	r->SetShaderKey("animshader");
	r->SetAnimator(m_pAnimComp);    // アニメーターをセット

	// 足音用コンポーネント追加
	m_pSoundEmitter = AddComponent<SoundEmitterComponent>("SoundEmitter");

	auto* throwAudio = AddComponent<ThrowAudioComponent>("ThrowAudio");
	m_pThrowComp->SetThrowEventListener(throwAudio);

	//SetScale(Vector3(10.0f, 10.0f, 10.0f));
}

void Player::Start(void)
{
	// 初期アニメを確定
	if (m_pAnimComp)
	{
		m_pAnimComp->Play(AnimType::Idle, ANIM_BLEND_TIME_SEC);
		m_pAnimComp->SetPlaybackSpeed(ANIM_SPEED_IDLE);
	}

	// カメラ初期化
	if (m_pCamera)
	{
		// 1) 最初の注視点を確定
		Vector3 pos = GetPosition();
		Vector3 lookAt = pos;
		lookAt.y += CAMERA_LOOK_AT_HEIGHT_NORMAL;
		m_pCamera->SetLookAt(lookAt);

		// 2) 方位角をプレイヤー向きに揃える（recenter相当を “代入で” やる）
		Vector3 f = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), m_Transform.GetRotation());
		f.y = 0.0f;
		if (f.LengthSquared() > 1e-6f) f.Normalize();
		const float yaw = std::atan2(-f.x, -f.z);

		m_CamAzimuth = yaw;
		m_CamElevation = CAMERA_ELEVATION_RECENTER;

		// 3) 補間用 “現在値” もターゲットで初期化
		m_CamRadiusCur = CAMERA_RADIUS_NORMAL;
		m_CamShoulderCur = CAMERA_SHOULDER_NORMAL;
		m_CamLookAtHeightCur = CAMERA_LOOK_AT_HEIGHT_NORMAL;
		m_CamNearCur = CAMERA_NEAR_NORMAL;

		// 4) CameraComponent 側も同じ値に揃える
		m_pCamera->SetMode(CameraComponent::Mode::Orbit);
		m_pCamera->SetRadius(m_CamRadiusCur);
		m_pCamera->SetShoulderOffset(m_CamShoulderCur);
		m_pCamera->SetNearPlane(m_CamNearCur);
		m_pCamera->SetAzimuth(m_CamAzimuth);
		m_pCamera->SetElevation(m_CamElevation);

		// 5) 衝突 pivot / ignore は最初から入れる
		Vector3 pivot = pos;
		pivot.y += m_CamLookAtHeightCur;
		m_pCamera->SetCollisionPivot(pivot);

		if (m_pCharVirtual)
			m_pCamera->SetIgnoreBody(m_pCharVirtual->GetInnerBodyID());
	}

	// 足音系の初期化（Updateで使う状態）
	m_Footstep.timer = 0.0f;
	m_Footstep.wasOnGround = (m_pCharVirtual) ? m_pCharVirtual->IsOnGround() : false;

	m_PrevAiming = false;
	m_PrevRightTrigger = 0.0f;
}

Player::InputState Player::ReadInputState(float dt)
{
	(void)dt;

	auto& input = CDirectInput::GetInstance();

	InputState st{};
	if (m_ForceLookAt.active)
	{
		return st; // 入力ゼロ
	}

	Vector3 input_dir = Vector3::Zero;

	// ---- 1) 入力から移動方向を作る ----
	// キー入力は「押している/押していない」だけなので、方向の合成になる
	if (input.CheckKeyBuffer(DIK_W)) { input_dir.z += 1.0f; }
	if (input.CheckKeyBuffer(DIK_S)) { input_dir.z -= 1.0f; }
	if (input.CheckKeyBuffer(DIK_A)) { input_dir.x -= 1.0f; }
	if (input.CheckKeyBuffer(DIK_D)) { input_dir.x += 1.0f; }

	// パッド
	// ※ GetLeftStick() が -1..1 を返す前提（デッドゾーンは Input 側で処理済み想定）
	Vector2 stick = input.GetLeftStick();
	input_dir.x += stick.x;
	input_dir.z += stick.y;

	// 倒し具合（0..1） ※キーの斜め(sqrt2)もここで1に丸める
	const float len = std::sqrt(input_dir.x * input_dir.x + input_dir.z * input_dir.z);
	const float amount = std::clamp(len, 0.0f, 1.0f);

	st.inputDir = input_dir;
	st.amount = amount;

	// 構え：Tキー or 左トリガー
	st.aiming =
		input.CheckKeyBuffer(DIK_T) ||
		(input.GetLeftTrigger() >= AIM_TRIGGER_THRESHOLD);

	// リセンター：Kキー(トリガー) or 右スティック押し込み
	st.recenter =
		input.CheckKeyBufferTrigger(DIK_K) ||
		input.GetButtonTrigger(XINPUT_GAMEPAD_RIGHT_THUMB);

	// 投げ：Fキー(トリガー) or 右トリガー（押し込みの立ち上がり）
	const bool keyThrow = input.CheckKeyBufferTrigger(DIK_F);
	const float rt = input.GetRightTrigger();
	const bool padThrow = (rt >= THROW_TRIGGER_THRESHOLD) && (m_PrevRightTrigger < THROW_TRIGGER_THRESHOLD);
	m_PrevRightTrigger = rt;

	st.throwPressed = keyThrow || padThrow;

	// ジャンプ入力
	st.wantsJump = input.CheckKeyBuffer(DIK_SPACE);

	// しゃがみキー（CキーorBボタン）押下中かどうか
	st.isCrouching = input.CheckKeyBufferTrigger(DIK_C) || input.GetButtonTrigger(XINPUT_GAMEPAD_B);

	return st;
}

void Player::BuildMoveDirection(const InputState& in, Vector3& outMoveDir, float& outMoveAmount)
{
	outMoveDir = Vector3::Zero;

	// 構え中は移動を止める（体の向き変更だけ）
	float move_amount = in.amount;
	if (in.aiming && AIM_LOCK_MOVE)
	{
		move_amount = 0.0f;
	}
	outMoveAmount = move_amount;

	if (in.amount <= MOVE_AMOUNT_EPSILON)
		return;

	// 「カメラが見ている向き」から前/右を作る（逆になりにくい）
	Vector3 camF = Vector3::Zero;

	if (m_pCamera)
	{
		// カメラが見ている方向 = LookAt - Position
		camF = m_pCamera->GetLookAt() - m_pCamera->GetPosition();
		camF.y = 0.0f;
	}
	else
	{
		// 保険（カメラが無いとき）
		camF = Vector3(0.0f, 0.0f, 1.0f);
	}

	if (camF.LengthSquared() > 1e-6f) camF.Normalize();

	// 右ベクトル（座標系により Cross の順序で左右が入れ替わることがある）
	Vector3 camR = WORLD_UP.Cross(camF);
	if (camR.LengthSquared() > 1e-6f) camR.Normalize();

	// 入力(input_dir)をカメラ基準でワールドへ変換
	// input_dir.x: 左右, input_dir.z: 前後
	Vector3 move_world = camR * in.inputDir.x + camF * in.inputDir.z;

	if (move_world.LengthSquared() > 1e-6f)
	{
		// 方向だけ正規化
		outMoveDir = move_world;
		outMoveDir.Normalize();
	}
}

void Player::ApplyFacingRotation(const InputState& in, const Vector3& moveDir, float moveAmount)
{
	// 向き（回転だけ）
	// 進行方向へ向ける
	// 非構え時のみ進行方向へ
	if (!in.aiming && moveAmount > MOVE_AMOUNT_EPSILON && moveDir.LengthSquared() > 1e-6f)
	{
		float targetYaw = std::atan2(-moveDir.x, -moveDir.z);
		m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(WORLD_UP, targetYaw));
	}
}

void Player::ApplyStance(const InputState& in)
{
	// ---- 2) 姿勢（しゃがみ） ----
	// 姿勢切り替え
	if (in.isCrouching)
	{
		m_CrouchToggle = !m_CrouchToggle;
	}
	// 姿勢の切替は「移動」より先に決めておくと、速度係数や当たり判定が同フレームで揃う
	if (m_pCharVirtual)
	{
		m_pCharVirtual->SetStance(
			m_CrouchToggle ? CharacterVirtualComponent::Stance::Crouch
			: CharacterVirtualComponent::Stance::Stand
		);
	}
}

void Player::ApplyMoveToCharacterVirtual(const Vector3& moveDir, float moveAmount, bool wantsJump)
{
	// ---- 4) CharacterVirtual に入力を渡す ----
	// 方向 + 量を渡すことで「スティック倒し具合に応じた速度」が作れる
	if (m_pCharVirtual)
	{
		float amount = moveAmount;
		if (m_CrouchToggle)
		{
			amount *= 0.55f; // 0.4〜0.7で調整
		}

		m_pCharVirtual->SetMoveInput(moveDir, amount);

		if (wantsJump)
			m_pCharVirtual->RequestJump();
	}
}

void Player::UpdateMovementAnimation(const InputState& in)
{
	// ---- 3) アニメーション選択 ----
	// 構え中は ThrowComponent が StoneThrow を主導するので、ここでは触らない
	if (!m_pAnimComp) return;
	if (in.aiming) return;

	if (in.amount < MOVE_AMOUNT_TO_IDLE)
	{
		m_pAnimComp->Play(m_CrouchToggle ? AnimType::Crouch : AnimType::Idle, ANIM_BLEND_TIME_SEC);
		m_pAnimComp->SetPlaybackSpeed(ANIM_SPEED_IDLE);
	}
	else
	{
		if (m_CrouchToggle)
		{
			m_pAnimComp->Play(AnimType::CrouchWalk, ANIM_BLEND_TIME_SEC);
			const float speed = std::lerp(ANIM_SPEED_CROUCH_WALK_MIN, ANIM_SPEED_CROUCH_WALK_MAX, in.amount);
			m_pAnimComp->SetPlaybackSpeed(speed);
		}
		else
		{
			const bool isRun = (in.amount >= MOVE_AMOUNT_WALK_TO_RUN);
			m_pAnimComp->Play(isRun ? AnimType::Run : AnimType::Walk, ANIM_BLEND_TIME_SEC);
			const float speed = isRun
				? std::lerp(ANIM_SPEED_RUN_MIN, ANIM_SPEED_RUN_MAX, in.amount)
				: std::lerp(ANIM_SPEED_WALK_MIN, ANIM_SPEED_WALK_MAX, in.amount);
			m_pAnimComp->SetPlaybackSpeed(speed);
		}
	}
}

void Player::UpdateFootstep(float dt)
{
	if (!m_pCharVirtual || !m_FootstepEnabled || !m_pSoundEmitter)
		return;

	const bool  onGround = m_pCharVirtual->IsOnGround();
	const float horizontalSpeed = m_pCharVirtual->GetHorizontalSpeed();

	// ---- 移動判定ヒステリシス（止まり際の揺れ対策）----
	constexpr float MOVE_START = FOOTSTEP_MOVE_SPEED_THRESHOLD;       
	constexpr float MOVE_STOP = FOOTSTEP_MOVE_SPEED_THRESHOLD * 0.75f;

	if (!m_IsMoving) m_IsMoving = (horizontalSpeed >= MOVE_START);
	else                    m_IsMoving = (horizontalSpeed >= MOVE_STOP);

	const bool isMoving = m_IsMoving;

	// ---- 姿勢係数（しゃがみ等）----
	const float kInterval = m_pCharVirtual->GetFootstepIntervalCoeff();
	const float kRadius = m_pCharVirtual->GetFootstepRadiusCoeff();
	const float kLoudness = m_pCharVirtual->GetFootstepLoudnessCoeff();

	// ---- 速度を0..1へ ----
	const float speed01 = REMAP_01_CLAMP(horizontalSpeed, FOOTSTEP_SPEED_MIN, FOOTSTEP_SPEED_MAX);

	// ---- 走りほど間隔を短く（頻繁に）----
	float interval = std::lerp(FOOTSTEP_WALK_INTERVAL, FOOTSTEP_RUN_INTERVAL, speed01);
	interval *= kInterval; // しゃがみは遅く、等

	if (onGround && isMoving)
	{
		m_Footstep.timer += dt;
		if (m_Footstep.timer >= interval)
		{
			m_Footstep.timer = 0.0f;

			// 音量：しゃがみ小、歩き普通、走り大（速度で連続スケール）
			float vol = std::lerp(FOOTSTEP_VOL_MIN, FOOTSTEP_VOL_MAX, speed01);
			vol *= kLoudness;

			// 届く範囲：速度で増える＋姿勢係数
			float radius = std::lerp(FOOTSTEP_RADIUS_MIN, FOOTSTEP_RADIUS_MAX, speed01);
			radius *= kRadius;

			// AI向けの強さ
			float loud = vol;

			WorldSoundEvent ev{};
			ev.Position = GetPosition();
			ev.Type = SoundType::Footstep;
			ev.Volume = vol;
			ev.Loudness = loud;
			ev.Radius = radius;

			// 天候で足音の種類を変える→ここで固定ラベルを入れない
			// Playback側(MapLabel)に任せる
			ev.PlayLabel = SOUND_LABEL_MAX;

			m_pSoundEmitter->EmitSound(ev);
		}
	}
	else
	{
		// 止まった/空中：次の再開が変にならないようにリセット
		m_Footstep.timer = 0.0f;
	}

	// ---- 着地音 ----
	if (!m_Footstep.wasOnGround && onGround)
	{
		const Vector3 v = m_pCharVirtual->GetLinearVelocity();
		const float impact = std::max(0.0f, -v.y);

		if (impact > LANDING_IMPACT_THRESHOLD)
		{
			WorldSoundEvent ev{};
			ev.Position = GetPosition();
			ev.Type = SoundType::Footstep;

			// 落下速度でスケール
			const float land01 = std::clamp(impact / LANDING_LOUDNESS_DIV, 0.0f, 1.0f);
			ev.Volume = std::lerp(0.2f, 1.0f, land01);
			ev.Loudness = ev.Volume;
			ev.Radius = std::lerp(300.0f, LANDING_SOUND_RADIUS, land01);

			// 着地専用SEを鳴らしたいなら明示
			ev.PlayLabel = SOUND_LABEL_MAX;

			m_pSoundEmitter->EmitSound(ev);
		}
	}

	m_Footstep.wasOnGround = onGround;
}


void Player::UpdateCamera(float dt, const InputState& in)
{
	// ---- 7) 位置を使ってカメラ更新 ----
	Vector3 pos = m_Transform.GetPosition();
	auto& input = CDirectInput::GetInstance();

	if (!m_pCamera) { return; }

	// ---- 強制注視（発見演出）を最優先 ----
	if (m_ForceLookAt.active)
	{
		// 衝突の起点は「プレイヤー頭付近」を維持（今の設計に合わせる）
		Vector3 pivot = pos;
		pivot.y += m_CamLookAtHeightCur;   // 普段のカメラ高さを流用
		m_pCamera->SetCollisionPivot(pivot);

		if (m_pCharVirtual)
			m_pCamera->SetIgnoreBody(m_pCharVirtual->GetInnerBodyID());

		// ベースとなるカメラ位置（開始時の位置）
		const Vector3 baseCamPos =
			m_ForceLookAt.freezePos ? m_ForceLookAt.frozenCamPos : m_pCamera->GetPosition();

		// 注視点だけ敵へ滑らかに寄せる
		m_ForceLookAt.lookAtCur = SmoothToVec3(m_ForceLookAt.lookAtCur, m_ForceLookAt.target, dt, m_ForceLookAt.turnSpeed);

		// ---- ここから「遠いときだけ少し寄る」 ----
		constexpr float ZOOM_START_DIST = 1800.0f;   // これ以上遠いなら寄る（調整）
		constexpr float ZOOM_MAX_ADVANCE = 350.0f;   // 最大でこれだけ寄る（調整）
		constexpr float ZOOM_SPEED = 8.0f;           // 寄りの補間速度（調整）

		Vector3 toT = m_ForceLookAt.target - baseCamPos;
		float dist = toT.Length();

		float zoomTarget = 0.0f;
		if (dist > 1e-4f && dist > ZOOM_START_DIST)
		{
			// 遠いほど少し寄るが、最大量で止める
			zoomTarget = std::min(dist - ZOOM_START_DIST, ZOOM_MAX_ADVANCE);
		}

		// 現在の寄り量を補間
		m_ForceLookAt.zoomCur = SmoothTo(m_ForceLookAt.zoomCur, zoomTarget, dt, ZOOM_SPEED);

		// ターゲット方向へ寄せたカメラ位置を作る
		Vector3 camPos = baseCamPos;
		if (dist > 1e-4f)
		{
			Vector3 dir = toT / dist; // 正規化
			camPos = baseCamPos + dir * m_ForceLookAt.zoomCur;
		}

		// カメラ適用
		m_pCamera->SetMode(CameraComponent::Mode::Direct);
		m_pCamera->SetPosition(camPos);
		m_pCamera->SetLookAt(m_ForceLookAt.lookAtCur);
		return;
	}

	// リセンター：プレイヤーの前方ベクトルの方向にカメラを向けたい
	if (in.recenter)
	{
		Vector3 f = Vector3::Transform(Vector3(0.0f, 0.0f, -1.0f), m_Transform.GetRotation());
		f.y = 0.0f;
		if (f.LengthSquared() > 1e-6f) f.Normalize();

		float playerYaw = std::atan2(-f.x, -f.z);
		m_CamAzimuth = playerYaw;
		m_CamElevation = CAMERA_ELEVATION_RECENTER;
	}

	// ---- カメラズーム（肩越し） ----
	const float targetRadius = in.aiming ? CAMERA_RADIUS_AIM : CAMERA_RADIUS_NORMAL;
	const float targetShoulder = in.aiming ? CAMERA_SHOULDER_AIM : CAMERA_SHOULDER_NORMAL;
	const float targetLookAtH = in.aiming ? CAMERA_LOOK_AT_HEIGHT_AIM : CAMERA_LOOK_AT_HEIGHT_NORMAL;
	const float targetNear = in.aiming ? CAMERA_NEAR_AIM : CAMERA_NEAR_NORMAL;

	m_CamRadiusCur = SmoothTo(m_CamRadiusCur, targetRadius, dt, CAMERA_SMOOTH_SPEED);
	m_CamShoulderCur = SmoothTo(m_CamShoulderCur, targetShoulder, dt, CAMERA_SMOOTH_SPEED);
	m_CamLookAtHeightCur = SmoothTo(m_CamLookAtHeightCur, targetLookAtH, dt, CAMERA_SMOOTH_SPEED);
	m_CamNearCur = SmoothTo(m_CamNearCur, targetNear, dt, CAMERA_SMOOTH_SPEED);

	m_pCamera->SetNearPlane(m_CamNearCur);
	m_pCamera->SetShoulderOffset(m_CamShoulderCur);

	// 衝突判定の始点(pivot)をカメラへ渡す
	{
		Vector3 pivot = pos;
		pivot.y += m_CamLookAtHeightCur;
		m_pCamera->SetCollisionPivot(pivot);

		// 自分(プレイヤー)は無視（カメラが自分に当たって詰むのを防ぐ）
		if (m_pCharVirtual)
			m_pCamera->SetIgnoreBody(m_pCharVirtual->GetInnerBodyID());
	}

	if (in.aiming)
	{
		// 構え状態：カメラの向いてる方向にキャラを向けて、肩越しで後ろから見る
		Vector3 pf = Vector3::Transform(Vector3(0.0f, 0.0f, -1.0f), m_Transform.GetRotation());
		pf.y = 0.0f;
		if (pf.LengthSquared() > 1e-6f) pf.Normalize();
		float yaw = m_CamAzimuth;

		// マウスで回転（構え中は右クリック不要）
		{
			LONG dx = input.GetMouseStateData().lX;
			LONG dy = input.GetMouseStateData().lY;
			yaw += dx * CAMERA_MOUSE_SENSITIVITY;
			m_CamElevation -= dy * CAMERA_MOUSE_SENSITIVITY;
		}

		// パッド右スティックで回転したい場合
		{
			Vector2 rs = input.GetRightStick();
			constexpr float PAD_YAW_SPEED = 3.2f;
			constexpr float PAD_PITCH_SPEED = 2.2f;
			yaw += rs.x * PAD_YAW_SPEED * dt;
			m_CamElevation += rs.y * PAD_PITCH_SPEED * dt;
		}

		// 仰角制限
		const float limit = (PI / 2.0f) - CAMERA_ELEVATION_LIMIT_EPSILON_RAD;
		if (m_CamElevation > limit) m_CamElevation = limit;
		if (m_CamElevation < -limit) m_CamElevation = -limit;

		// 構え中は下を向きすぎない（見下ろしになりにくくする）
		if (m_CamElevation < AIM_ELEVATION_MIN_DOWN)
			m_CamElevation = AIM_ELEVATION_MIN_DOWN;

		// 体の向き更新（yawのみ）
		m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(WORLD_UP, yaw));
		m_CamAzimuth = yaw;

		// Direct：肩越し後方カメラ（上からではなく後ろから）
		m_pCamera->SetMode(CameraComponent::Mode::Direct);

		// yaw + elevation から forward を作る（Direct の視線方向）
		const float ce = std::cos(m_CamElevation);
		const float se = std::sin(m_CamElevation);
		const float ca = std::cos(m_CamAzimuth);
		const float sa = std::sin(m_CamAzimuth);

		Vector3 forward(-sa * ce, se, -ca * ce);
		if (forward.LengthSquared() > 1e-6f) forward.Normalize();

		Vector3 right = WORLD_UP.Cross(forward);
		if (right.LengthSquared() > 1e-6f) right.Normalize();

		Vector3 pivot = pos;
		pivot.y += m_CamLookAtHeightCur;

		Vector3 camPos = pivot - forward * m_CamRadiusCur + right * m_CamShoulderCur;
		Vector3 camLook = pivot + forward * AIM_LOOK_DISTANCE;

		m_pCamera->SetPosition(camPos);
		m_pCamera->SetLookAt(camLook);
	}
	else
	{
		// 通常：Orbit（今のやつ）
		m_pCamera->SetMode(CameraComponent::Mode::Orbit);

		// 右クリック中だけカメラ回転（よくあるTPS操作）
		if (input.GetMouseRButtonCheck())
		{
			LONG dx = input.GetMouseStateData().lX;
			LONG dy = input.GetMouseStateData().lY;

			float sensitivity = CAMERA_MOUSE_SENSITIVITY;

			m_CamAzimuth += dx * sensitivity;
			m_CamElevation -= dy * sensitivity;
		}

		// パッド右スティックでも回転したい場合
		{
			Vector2 rs = input.GetRightStick();
			constexpr float PAD_YAW_SPEED = 3.0f;
			constexpr float PAD_PITCH_SPEED = 2.2f;
			m_CamAzimuth += rs.x * PAD_YAW_SPEED * dt;
			m_CamElevation += rs.y * PAD_PITCH_SPEED * dt;
		}

		// 仰角の制限(-89°～89°の範囲に制限)
		const float limit = (PI / 2.0f) - CAMERA_ELEVATION_LIMIT_EPSILON_RAD;
		if (m_CamElevation > limit) m_CamElevation = limit;
		if (m_CamElevation < -limit) m_CamElevation = -limit;

		m_pCamera->SetRadius(m_CamRadiusCur);
		m_pCamera->SetAzimuth(m_CamAzimuth);
		m_pCamera->SetElevation(m_CamElevation);

		Vector3 lookAt = pos;
		lookAt.y += m_CamLookAtHeightCur;  // 注視点を少し上にずらす
		m_pCamera->SetLookAt(lookAt);
	}
}

void Player::UpdateThrowNotify(const InputState& in)
{
	if (!m_pThrowComp)
		return;

	// 構え開始/終了を通知（敵AIも同じ通知で動かせる）
	if (in.aiming && !m_PrevAiming)
		m_pThrowComp->OnAimStart();
	if (!in.aiming && m_PrevAiming)
		m_pThrowComp->OnAimEnd();

	// 投げ押下
	if (in.throwPressed)
		m_pThrowComp->Throw(ThrowItemId::Rock);

	m_PrevAiming = in.aiming;
}


// プレイヤー更新
void Player::Update(const float deltatime)
{
	// 1) 入力を読む
	const InputState in = ReadInputState(deltatime);

	// 2) Throw への通知（構え開始/終了/投げ押下）
	UpdateThrowNotify(in);

	// 3) 移動入力をカメラ基準の move_dir に変換
	Vector3 move_dir = Vector3::Zero;
	float move_amount = 0.0f;
	BuildMoveDirection(in, move_dir, move_amount);

	// 4) 向き（非構え時のみ移動方向へ）
	ApplyFacingRotation(in, move_dir, move_amount);

	// 5) 姿勢（しゃがみ）
	ApplyStance(in);

	// 6) CharacterVirtual に入力を渡す
	ApplyMoveToCharacterVirtual(move_dir, move_amount, in.wantsJump);

	// 7) アニメ（通常時のみ）
	UpdateMovementAnimation(in);

	// 8) 足音
	UpdateFootstep(deltatime);

	// 9) カメラ
	UpdateCamera(deltatime, in);
}

void Player::Draw(void) const
{
}

void Player::Uninit(void)
{
}

void Player::GetVisibilitySamplePoints(const Vector3& eyePos, std::vector<Vector3>& out) const
{
	out.clear();

	auto* ch = m_pCharVirtual;
	if (!ch)
	{
		// 最低限、頭くらいの 1 点だけでも返す
		out.push_back(GetPosition() + Vector3(0.0f, VISIBILITY_FALLBACK_HEAD_HEIGHT, 0.0f));
		return;
	}

	float hh = ch->GetCurrentHalfHeight();
	float r = ch->GetRadius();

	Vector3 foot = GetPosition();   // 足元（カプセルの下端近辺）
	float topY = 2.0f * (hh + r);   // カプセルの上端近辺
	float midY = 0.5f * topY;

	Vector3 centerMid = foot + Vector3(0.0f, midY, 0.0f);
	Vector3 top = foot + Vector3(0.0f, topY - r, 0.0f);     // 頭寄り
	Vector3 bottom = foot + Vector3(0.0f, r, 0.0f);         // 足寄り

	// 敵から見た左右方向を決める
	// eyePos から中心を見た方向を基準に「左右」を作る
	Vector3 viewDir = centerMid - eyePos;
	if (viewDir.LengthSquared() < VISIBILITY_EPSILON_SQ)
	{
		viewDir = VISIBILITY_DEFAULT_VIEW_DIR;
	}
	viewDir.Normalize();

	Vector3 up = WORLD_UP;
	Vector3 side = viewDir.Cross(up);
	if (side.LengthSquared() < VISIBILITY_EPSILON_SQ)
	{
		side = VISIBILITY_DEFAULT_SIDE_DIR;
	}
	side.Normalize();

	// 中心線
	out.push_back(top);
	out.push_back(bottom);

	// 左右（楕円の横方向）
	// 半径ぶん左右へずらして、遮蔽判定の精度を上げる
	out.push_back(centerMid + side * r);
	out.push_back(centerMid - side * r);
}

JPH::BodyID Player::GetInnerBodyID(void) const
{
	if (m_pCharVirtual)
	{
		return m_pCharVirtual->GetInnerBodyID();
	}
	return JPH::BodyID();
}

void Player::StartForceLookAt(const Vector3& targetWorld, float turnSpeed, bool freezePos)
{
	if (!m_pCamera) return;

	m_ForceLookAt.active = true;
	m_ForceLookAt.freezePos = freezePos;
	m_ForceLookAt.turnSpeed = std::max(0.0f, turnSpeed);
	m_ForceLookAt.target = targetWorld;

	// 今のカメラ状態を起点にする（「開始した瞬間にガクッ」を防ぐ）
	m_ForceLookAt.lookAtCur = m_pCamera->GetLookAt();
	m_ForceLookAt.frozenCamPos = m_pCamera->GetPosition();
	m_ForceLookAt.zoomCur = 0.0f;
}

void Player::StopForceLookAt()
{
	m_ForceLookAt.active = false;
}
