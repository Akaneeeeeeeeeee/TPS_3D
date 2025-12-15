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

#include <algorithm> // std::clamp
#include <cmath>     // std::sqrt, std::atan2, std::lerp

namespace
{
	//============================
	// Player: 形状 / 初期設定
	//============================

	// プレイヤーカプセル（単位はあなたのワールド単位）
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
	constexpr float ANIM_SPEED_CROUCH_WALK_MAX = 1.10f;

	// 歩き：スティック量(0..1) → 再生速度
	constexpr float ANIM_SPEED_WALK_MIN = 0.80f;
	constexpr float ANIM_SPEED_WALK_MAX = 1.20f;

	// 走り：スティック量(0..1) → 再生速度
	constexpr float ANIM_SPEED_RUN_MIN = 0.90f;
	constexpr float ANIM_SPEED_RUN_MAX = 1.50f;

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
	// ※ここは実測値に合わせて調整する。初期値は仮（推測です）
	// horizontalSpeed が FOOTSTEP_SPEED_MIN のとき最小、FOOTSTEP_SPEED_MAX で最大になる。
	constexpr float FOOTSTEP_SPEED_MIN = 60.0f;   // (推測です) 歩き時の水平速度の目安
	constexpr float FOOTSTEP_SPEED_MAX = 250.0f;  // (推測です) 走り時の水平速度の目安

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

#ifdef _DEBUG
	//============================
	// デバッグ用
	//============================

	// デバッグ用スロー再生（TimeScale）
	constexpr float DEBUG_TIME_SCALE_SLOW = 0.10f;

	// デバッグでEmitする音（半径/音量）
	constexpr float DEBUG_SOUND_RADIUS = 1000.0f;
	constexpr float DEBUG_SOUND_LOUDNESS = 1.0f;
#endif
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
		m_pCharaVirtualComp->SetCapsule(PLAYER_CAPSULE_HALF_HEIGHT, PLAYER_CAPSULE_RADIUS);
	}

	// TPS カメラコンポーネント追加
	{
		m_pCamera = this->AddComponent<CameraComponent>(m_Name + "_CameraComponent");

		// 必要なら初期パラメータを調整
		m_pCamera->SetRadius(CAMERA_RADIUS);
	}
}

// プレイヤー更新
void Player::Update(const float deltatime)
{
	auto& input = CDirectInput::GetInstance();

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
	// len: 入力方向の長さ。0に近いと正規化できないので後で epsilon 判定する
	const float len = std::sqrt(input_dir.x * input_dir.x + input_dir.z * input_dir.z);
	const float amount = std::clamp(len, 0.0f, 1.0f);

#ifdef _DEBUG
	if (input.CheckKeyBuffer(DIK_K)) {
		Time::GetInstance().SetTimeScale(DEBUG_TIME_SCALE_SLOW);
	}
	// E キーが押された瞬間に音を出す
	// ※今は押している間ずっと出るので、Trigger にしたいなら CheckKeyBufferTrigger を使う
	if (input.CheckKeyBuffer(DIK_E))
	{
		WorldSoundEvent ev{};
		ev.Position = GetPosition();
		ev.Radius = DEBUG_SOUND_RADIUS;
		ev.Loudness = DEBUG_SOUND_LOUDNESS;
		ev.Type = SoundType::Footstep; // とりあえず足音扱い

		SoundManager::GetInstance().EmitSound(ev);
	}
#endif // _DEBUG

	// ジャンプ入力
	bool wants_jump = input.CheckKeyBuffer(DIK_SPACE);

	// move_dir は「方向だけ」持つ（速さは amount で別に持つ）
	// ※ amount が小さいときに len で割ると危ないので epsilon 判定する
	Vector3 move_dir = Vector3::Zero;
	if (amount > MOVE_AMOUNT_EPSILON)
	{
		// 方向だけ正規化
		move_dir = Vector3(input_dir.x / len, 0.0f, input_dir.z / len);

		// 向き（回転だけ）
		// 進行方向へ向ける（座標系に合わせて符号は調整）
		float targetYaw = std::atan2(-move_dir.x, -move_dir.z);
		m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(WORLD_UP, targetYaw));
	}

	// しゃがみキー（Cキー）押下中かどうか
	const bool isCrouching = input.CheckKeyBuffer(DIK_C);

	// ---- 2) 姿勢（しゃがみ） ----
	// 姿勢の切替は「移動」より先に決めておくと、速度係数や当たり判定が同フレームで揃う
	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetStance(
			isCrouching ? CharacterVirtualComponent::Stance::Crouch
			: CharacterVirtualComponent::Stance::Stand
		);
	}

	// ---- 4) CharacterVirtual に入力を渡す ----
	// 方向 + 量を渡すことで「スティック倒し具合に応じた速度」が作れる
	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetMoveInput(move_dir, amount);

		if (wants_jump)
			m_pCharaVirtualComp->RequestJump();
	}

	// ---- 3) アニメーション選択 ----
	// 閾値でモーション（Idle/Walk/Run）を切り替え、再生速度は amount をそのまま反映する
	// ※「歩き/走りの見た目」が合わない場合は、速度の範囲(～MAX)を調整する
	if (m_pAnimComp)
	{
		if (amount < MOVE_AMOUNT_TO_IDLE)
		{
			// 入力がほぼ無い → 待機
			m_pAnimComp->Play(isCrouching ? AnimType::Crouch : AnimType::Idle, ANIM_BLEND_TIME_SEC);
			m_pAnimComp->SetPlaybackSpeed(ANIM_SPEED_IDLE);
		}
		else
		{
			// 入力あり → 移動モーション
			if (isCrouching)
			{
				// しゃがみ歩き
				m_pAnimComp->Play(AnimType::CrouchWalk, ANIM_BLEND_TIME_SEC);

				// 再生速度は倒し具合に応じて連続変化
				const float speed = std::lerp(ANIM_SPEED_CROUCH_WALK_MIN, ANIM_SPEED_CROUCH_WALK_MAX, amount);
				m_pAnimComp->SetPlaybackSpeed(speed);
			}
			else
			{
				// 立ち：閾値で Walk/Run を切替
				const bool isRun = (amount >= MOVE_AMOUNT_WALK_TO_RUN);
				m_pAnimComp->Play(isRun ? AnimType::Run : AnimType::Walk, ANIM_BLEND_TIME_SEC);

				// 再生速度は「選んだクリップ」に対して倒し具合を反映
				// （歩きクリップに走り速度を入れると足が滑りやすいので分ける）
				const float speed = isRun
					? std::lerp(ANIM_SPEED_RUN_MIN, ANIM_SPEED_RUN_MAX, amount)
					: std::lerp(ANIM_SPEED_WALK_MIN, ANIM_SPEED_WALK_MAX, amount);

				m_pAnimComp->SetPlaybackSpeed(speed);
			}
		}
	}

	// ---- 5) コンポーネント更新（ここで位置が決まる）----
	// CharacterVirtualComponent の Update で物理更新され、Owner の位置が確定する
	GameObject::Update(deltatime);

	// ---- 6) CharacterVirtual の接地判定に同期して足音を出す ----
	// 「移動している + 地面にいる」時だけ、一定間隔で音イベントを発行する
	if (m_pCharaVirtualComp && m_FootstepEnabled)
	{
		bool  onGround = m_pCharaVirtualComp->IsOnGround();
		float horizontalSpeed = m_pCharaVirtualComp->GetHorizontalSpeed();

		bool isMoving = horizontalSpeed > FOOTSTEP_MOVE_SPEED_THRESHOLD;

		if (onGround && isMoving)
		{
			// 姿勢ごとの係数を取得（しゃがみは足音が小さく/遅く/狭くなる等）
			float kInterval = m_pCharaVirtualComp->GetFootstepIntervalCoeff();
			float kRadius = m_pCharaVirtualComp->GetFootstepRadiusCoeff();
			float kLoudness = m_pCharaVirtualComp->GetFootstepLoudnessCoeff();

			// 実際の足音間隔 = 立ち基準 * 姿勢係数
			float interval = FOOTSTEP_BASE_INTERVAL * kInterval;

			m_FootstepTimer += deltatime;

			if (m_FootstepTimer >= interval)
			{
				m_FootstepTimer = 0.0f;

				// ---- 速度から「音の大きさ/届く範囲係数」を作る ----
				// horizontalSpeed: CharacterVirtual から取った実測の水平速度
				// speed01: FOOTSTEP_SPEED_MIN～MAX を 0..1 に押し込める
				const float speed01 = REMAP_01_CLAMP(horizontalSpeed, FOOTSTEP_SPEED_MIN, FOOTSTEP_SPEED_MAX);

				// 速度が遅いほど小さく、速いほど大きく
				const float speedLoudnessScale =
					std::lerp(FOOTSTEP_SPEED_LOUDNESS_SCALE_MIN, FOOTSTEP_SPEED_LOUDNESS_SCALE_MAX, speed01);

				// 半径も速度で変える
				const float speedRadiusScale =
					std::lerp(FOOTSTEP_SPEED_RADIUS_SCALE_MIN, FOOTSTEP_SPEED_RADIUS_SCALE_MAX, speed01);

				// ---- 足音の WorldSoundEvent を飛ばす ----
				WorldSoundEvent ev{};
				ev.Position = GetPosition();

				// 届く範囲：基準 * 姿勢係数 * 速度係数
				ev.Radius = FOOTSTEP_BASE_RADIUS * kRadius * speedRadiusScale;

				// AIが感じる強さ：基準 * 姿勢係数 * 速度係数
				ev.Loudness = FOOTSTEP_BASE_LOUDNESS * kLoudness * speedLoudnessScale;

				// 実際の再生音量も連動（不要なら固定 1.0f でもよい）
				ev.Volume = 1.0f * speedLoudnessScale;

				ev.Type = SoundType::Footstep;
				SoundManager::GetInstance().EmitSound(ev);
			}
		}
		else
		{
			// 空中や停止中はタイマーをリセット
			// ※停止→すぐ移動再開で「変な間隔」になりにくい
			m_FootstepTimer = 0.0f;
		}

		// 着地音をつけたいならここで「前フレーム非接地 → 今フレーム接地」を見る
		// ※落下ダメージやカメラ揺れなどのトリガーにも使える
		if (!m_WasOnGround && onGround)
		{
			// 着地時の縦速度から強さを決めるなども可能
			Vector3 v = m_pCharaVirtualComp->GetLinearVelocity();
			float   vy = v.y;
			float   impact = std::max(0.0f, -vy); // 下向き速度

			// そこそこ高いところから落ちたときだけ
			if (impact > LANDING_IMPACT_THRESHOLD)
			{
				WorldSoundEvent ev{};
				ev.Position = GetPosition();
				ev.Radius = LANDING_SOUND_RADIUS;
				ev.Loudness = std::clamp(
					impact / LANDING_LOUDNESS_DIV,
					LANDING_LOUDNESS_MIN,
					LANDING_LOUDNESS_MAX
				);
				ev.Type = SoundType::Footstep;

				SoundManager::GetInstance().EmitSound(ev);
			}
		}

		m_WasOnGround = onGround;
	}

	// ---- 7) 位置を使ってカメラ更新 ----
	// ここまででプレイヤー位置が更新されている前提
	Vector3 pos = m_Transform.GetPosition();

	if (m_pCamera)
	{
		// static にすると「初回の値を保持」する（オブジェクト複数生成時は注意）
		// ※ Player が1体だけなら問題になりにくいが、将来的にはメンバ変数にするのが安全
		static float azimuth = m_pCamera->GetAzimuth();
		static float elevation = m_pCamera->GetElevation();

		// 右クリック中だけカメラ回転（よくあるTPS操作）
		if (input.GetMouseRButtonCheck())
		{
			LONG dx = input.GetMouseStateData().lX;
			LONG dy = input.GetMouseStateData().lY;

			// マウス感度
			float sensitivity = CAMERA_MOUSE_SENSITIVITY;

			// マウスの移動量に応じてカメラの角度を更新
			azimuth += dx * sensitivity;
			elevation -= dy * sensitivity;

			// 仰角の制限(-89°～89°の範囲に制限)
			const float limit = (PI / 2.0f) - CAMERA_ELEVATION_LIMIT_EPSILON_RAD;
			if (elevation > limit) elevation = limit;
			if (elevation < -limit) elevation = -limit;
		}

		// 半径や角度は毎フレーム反映（今後ズーム等を入れるならここを拡張）
		m_pCamera->SetRadius(CAMERA_RADIUS);
		m_pCamera->SetAzimuth(azimuth);
		m_pCamera->SetElevation(elevation);

		// TPS なのでカメラはプレイヤーから一定距離離れる
		Vector3 lookAt = pos;
		lookAt.y += CAMERA_LOOK_AT_HEIGHT;  // 注視点を少し上にずらす
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
		out.push_back(GetPosition() + Vector3(0.0f, VISIBILITY_FALLBACK_HEAD_HEIGHT, 0.0f));
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
	out.push_back(centerMid);
	out.push_back(top);
	out.push_back(bottom);

	// 左右（楕円の横方向）
	// 半径ぶん左右へずらして、遮蔽判定の精度を上げる
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
