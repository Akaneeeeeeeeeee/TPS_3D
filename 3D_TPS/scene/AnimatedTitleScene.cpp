#include <string>
#include <array>

#include "system/Framework/Application/Entry/main.h"
#include "system/CDirectInput.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/DebugUI.h"
#include "system/utility.h"
#include "system/AimOrientation.h"
#include "AnimatedTitleScene.h"
#include "Framework/GameObject/Player/Player.h"
#include "Framework/GameObject/Player/TitlePlayerActor.h"
#include "Framework/GameObject/Enemy/Enemy.h"
#include "Framework/GameObject/WeatherController/WeatherController.h"
#include "system/GameObject/obstacle.h"
#include "Framework/GameObject/Terrain/Terrain.h"
#include "system/GameObject/Skydome.h"
#include "Framework/SoundManager/SoundManager.h"
#include "Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/GameObject/StreetLight/StreetLight.h"
#include "Framework/Component/UI/UIImageComponent.h"

// 指定方向ベクトルから Yaw 回転を求める（XZ平面投影、正規化済み前提）
static Quaternion YawQuatFromDirXZ(const Vector3& dir)
{
	Vector3 d = dir;
	d.y = 0.0f;
	if (d.LengthSquared() < 1e-6f)
		return Quaternion::Identity;

	d.Normalize();
	const float yaw = std::atan2(-d.x, -d.z);
	return Quaternion::CreateFromAxisAngle(Vector3::Up, yaw);
}

// 平行光源の方向セット
void AnimatedTitleScene::debugDirectionalLight()
{
	static Vector4 direction = Vector4(0.0f, 0.0f, 1.0f, 0.0f); // Z軸+方向に光を当てる	

	ImGui::Begin("debug Directional Light");

	ImGui::SliderFloat3("direction ", &direction.x, -1, 1);
	direction.Normalize();										// 正規化

	LIGHT light{};
	light.Enable = true;
	light.Direction = direction;

	light.Direction.Normalize();
	light.Ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
	light.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);

	Vector4 Direction = Vector4(direction.x, direction.y, direction.z, 0.0f);
	Renderer::SetLight(light);

	ImGui::End();
}

// デバッグフリーカメラ
void AnimatedTitleScene::debugFreeCamera()
{
	ImGui::Begin("debug Free camera");

	static float radius = 100.0f;
	static Vector3 pos = Vector3(0, 0, radius);
	static Vector3 lookat = Vector3(0, 0, 0);
	static float elevation = -90.0f * PI / 180.0f;
	static float azimuth = PI / 2.0f;

	static Vector3 spherecenter = Vector3(0, 0, 0);

	ImGui::SliderFloat("Radius", &radius, 1, 800);
	ImGui::SliderFloat("Elevation", &elevation, -PI, PI);
	ImGui::SliderFloat("Azimuth", &azimuth, -PI, PI);

	ImGui::SliderFloat3("lookat ", &lookat.x, -100, 100);

	// カメラの位置を極座標からデカルト座標に変換
	m_camera.SetRadius(radius);
	m_camera.SetElevation(elevation);
	m_camera.SetAzimuth(azimuth);
	m_camera.SetLookat(lookat);

	// カメラの位置を極座標から求める
	m_camera.CalcCameraPosition();

	ImGui::End();
}

/**
 * @brief コンストラクタ
 */
AnimatedTitleScene::AnimatedTitleScene() : IScene()
{
	m_NextSceneName = "CollisionTestScene";
}

/**
 * @brief クリアシーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void AnimatedTitleScene::Update(const float deltatime)
{
	m_TitleScript.Tick(deltatime);      // 先に台本で MoveDir/Face/Anim を決める
	m_pObjectManager->Update(deltatime);

#ifdef _DEBUG
	// ImGui操作中に「何か入力」で開始してしまうのを防ぐ
	if (ImGui::GetCurrentContext())
	{
		const auto& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantCaptureMouse)
			return;
	}
	
	// デバッグ中はEnterキーで次シーンへ
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_RETURN))
	{
		SetChangeScene(true);
		SetNextSceneName(m_NextSceneName);
		return;
	}
#endif

	// 何か入力されたら即ゲーム開始（次シーンへ）
	if (CDirectInput::GetInstance().AnyInputTriggered())
	{
		SetChangeScene(true);
		SetNextSceneName(m_NextSceneName);
		return;
	}

	// 入力が無い場合は、演出が終わったら自動遷移
	if (m_TitleScript.IsFinished())
	{
		SetChangeScene(true);
		SetNextSceneName(m_NextSceneName);
	}
}


/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void AnimatedTitleScene::Draw(void)
{
	// 3軸カラー
	Color axiscol[3] = {
		Color(1, 0, 0, 1),
		Color(0, 1, 0, 1),
		Color(0, 1, 1, 1)
	};

	Vector3 sp;
	sp = m_Player->GetTransform().GetPosition();
	sp.y -= 500.0f;

#ifdef _DEBUG
	// ワールド軸を描画
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}
	SetLineWidth(3.0f);
	LineDrawerDraw(1000, sp, Vector3(0, 1, 0), Color(1, 1, 0, 1));
#endif
	m_pObjectManager->Draw();
}

/**
 * @brief シーンの初期化処理
 */
void AnimatedTitleScene::Init(ObjectManager* _Mgr)
{
	// オブジェクトマネージャのセット
	this->m_pObjectManager = _Mgr;
	
	m_Terrain = m_pObjectManager->Instantiate<Terrain>("city", Tag::Field);
	m_Terrain->SetPosition(Vector3(0.0f, -100.0f, 0.0f));
	m_Terrain->SetScale(Vector3(100.0f, 100.0f, 100.0f));
	m_Terrain->SetScene(this);

	// プレイヤー追加
	m_Player = m_pObjectManager->Instantiate<TitlePlayerActor>("PlayerActor", Tag::Player);
	m_Player->SetPosition(Vector3(1000.0f, 100.0f, -3350.0f));
	m_Player->SetTerrain(m_Terrain);

	// スカイドーム
	auto skydome = m_pObjectManager->Instantiate<Skydome>("skydome", Tag::Object);
	skydome->SetTexture("assets/texture/test.jpg");
	//skydome->SetTexture("assets/texture/haikei.jpg");

	// 天候オブジェクト
	m_Weather = m_pObjectManager->Instantiate<WeatherController>("WeatherController", Tag::Object);
	m_Weather->SetPosition(Vector3(1920.0f, 750.0f, -3500.0f));

	// 敵追加
	m_Enemy = m_pObjectManager->Instantiate<Enemy>("Enemy", Tag::Enemy);
	m_Enemy->SetTerrain(m_Terrain);
	m_Enemy->SetPosition(Vector3(2000.0f, 0.0f, -3350.0f));
	m_Enemy->SetWayPoints(Vector3(2000.0f, 0.0f, -3500.0f), Vector3(4000.0f, 0.0f, -3350.0f));

	// 障害物追加
	m_Obstacles[0] = m_pObjectManager->Instantiate<obstacle>("Obstacle1", Tag::Object);
	m_Obstacles[0]->SetPosition(Vector3(1500.0f, -50.0f, -3450.0f));
	// カメラに平行に配置
	m_Obstacles[0]->SetRotation(Quaternion::CreateFromAxisAngle(Vector3::Up, 0.75f));
	m_Obstacles[0]->SetScale(Vector3(175.0f, 65.0f, 25.0f));

	auto* lightObj = m_pObjectManager->Instantiate<StreetLight>("StreetLight", Tag::Light, Transform::One());
	lightObj->SetPosition(Vector3(1900.0f, 100.0f, -3650.0f));
	// 地面の明るい円半径を直接指定
	lightObj->SetGroundCircle(
		/*groundRadius=*/600.0f,
		/*groundY=*/0.0f,
		/*topRadiusMin=*/80.0f,   // 上面の“口径”を確保
		/*innerRatio=*/0.6f       // 中心が強い範囲
	);

	// 台本セットアップ
	m_TitleScript.Setup(m_Player, m_Enemy, m_Obstacles[0]);

	// タイトル画像の生成
	m_TitleImage = std::make_unique<CSprite>(SCREEN_WIDTH, SCREEN_HEIGHT, "assets/texture/Images/SilentEchoT1.png");

	// UI専用オブジェクト（Tagは何でもOK）
	auto* uiObj = m_pObjectManager->Instantiate<GameObject>(
		"UI_TitleLogo",
		Tag::Object,              // Tag::UI があるならそれでもOK
		Transform::One()
	);

	UIImageComponent* img = uiObj->AddComponent<UIImageComponent>(
		"Image",
		"assets/texture/Images/SilentEchoT1.png"
	);

	// UITransform（内包Rect）をセット
	auto& r = img->Rect();

	// 画面中央に置く
	r.anchor = { 1.0f, 0.f };          // 基準点：画面中央
	r.anchoredPosPx = { 0.0f, 0.0f };   // 基準点からの差分(px)
	r.sizePx = { 800.0f, 400.0f };      // 表示サイズ(px)
	r.pivot = { 0.5f, 0.5f };          // 自分の中心を基準
	r.rotZRad = 0.0f;

	r.layer = 100;
	r.order = 0;
	r.visible = true;

#ifdef _DEBUG
	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));
#endif
}

/**
 * @brief シーンの終了処理
 */
void AnimatedTitleScene::Uninit()
{
	//m_pObjectManager->DestroySceneObjects(m_CurrentSceneName);
	this->SetChangeScene(false);
}


/*
* @brief タイトル演出台本セットアップ
*/
void TitleScript::Setup(TitlePlayerActor* player, Enemy* enemy, obstacle* cover)
{
	m_Player = player;
	m_Enemy = enemy;
	m_Cover = cover;

	// 位置は「障害物基準」にするとマジックナンバーが減る
	const Vector3 c = m_Cover->GetPosition();
	const Vector3 s = m_Cover->GetScale();

	m_EnterPos = Vector3(c.x - s.x * 1.2f, c.y, c.z + s.z * 2.0f);		// 左手前から入る
	m_HidePos = Vector3(c.x + s.x * 0.5f, c.y, c.z - s.z * 5.0f);		// 障害物の右側に隠れる
	m_ExitPos = Vector3(1500.0f, c.y, -4250.0f);	// 右側に抜ける

	m_RockLandPos = Vector3(c.x + s.x * 1.8f, c.y, c.z + s.z * 0.8f);	// 石の着地点（敵を誘導したい場所）

	m_State = State::EnterMove;
	m_Timer = 0.0f;
}

void TitleScript::Tick(float dt)
{
	if (!m_Player) { return; }

	m_Timer += dt;

	switch (m_State)
	{
	case State::EnterMove:
		MoveTo(m_HidePos, /*run=*/false, /*sideways=*/true);
		if (ArrivedXZ(m_HidePos, 50.0f)) Enter(State::HideIdle);
		break;

	case State::HideIdle:
		StopIdle();
		if (m_Timer >= 1.0f) Enter(State::LookAround);
		break;

	case State::LookAround:
	{
		StopAndCheckOverWall();

		// “見た”タイミングっぽいところで1回だけ判定（数値は調整）
		if (!m_Scanned && m_Timer >= 0.35f)
		{
			m_Scanned = true;

			if (IsEnemyNear(700.0f))
			{
				Enter(State::ThrowRock);
			}
		}

		// 敵がいないなら一定時間で退場
		if (m_Timer >= 1.2f)
		{
			Enter(State::ExitMove);
		}

		break;
	}

	case State::ThrowRock:
		StopIdle();
		FaceTo(m_RockLandPos);
		if (!m_Thrown)
		{
			m_Thrown = true;
			EmitRockSound(m_RockLandPos);
		}
		// 投擲アニメ時間ぶん待つ
		if (m_Timer >= 0.6f) Enter(State::ExitMove);
		break;

	case State::ExitMove:
		MoveTo(m_ExitPos, /*run=*/true, /*sideways=*/false);

		if (ArrivedXZ(m_ExitPos, 80.0f))
		{
			Enter(State::ReEnter);
		}
		break;

	case State::ReEnter:
	{
		// 入口へワープ（向きは HidePos 方向に向ける）
		const Vector3 dir = (m_HidePos - m_EnterPos);
		const Quaternion rot = YawQuatFromDirXZ(dir);

		m_Player->SetPose(m_EnterPos, rot);

		// 1フレーム目に変な入力が残らないようにゼロクリア
		m_Player->SetMove(Vector3::Zero, 0.0f);
		m_Player->SetTargetAnim(TitlePlayerActor::TitleAnim::Idle);
		m_Player->SetSidewaysRight(false);

		// 次フレームからまた入場演出
		Enter(State::EnterMove);
		break;
	}


	}
}

void TitleScript::Enter(State next)
{
	m_State = next;
	m_Timer = 0.0f;
	if (next == State::ThrowRock) m_Thrown = false;
	if (next == State::LookAround) m_Scanned = false;
}

Vector3 TitleScript::DirXZ(const Vector3& from, const Vector3& to)
{
	Vector3 d = to - from;
	d.y = 0.0f;
	if (d.LengthSquared() < 1e-6f) return Vector3::Zero;
	d.Normalize();
	return d;
}

bool TitleScript::ArrivedXZ(const Vector3& target, float r) const
{
	Vector3 p = m_Player->GetPosition();
	Vector3 d = target - p;
	d.y = 0.0f;
	return d.LengthSquared() <= (r * r);
}

void TitleScript::MoveTo(const Vector3& target, bool run, bool sideways)
{
	const Vector3 dir = DirXZ(m_Player->GetPosition(), target);
	const float amount = run ? 1.0f : 0.7f;

	m_Player->SetMove(dir, amount);
	m_Player->SetTargetAnim(run ? TitlePlayerActor::TitleAnim::Run : TitlePlayerActor::TitleAnim::Walk);

	// 進行方向に向く / 横移動演出も可能
	m_Player->SetFaceMode(TitlePlayerActor::FaceMode::FaceMoveDir);
	m_Player->SetSidewaysRight(sideways);
}

void TitleScript::StopIdle()
{
	m_Player->SetMove(Vector3::Zero, 0.0f);
	m_Player->SetTargetAnim(TitlePlayerActor::TitleAnim::Idle);
	m_Player->SetSidewaysRight(true);
}

void TitleScript::FaceTo(const Vector3& targetPos)
{
	m_Player->SetFaceMode(TitlePlayerActor::FaceMode::FaceTargetPos);
	m_Player->SetFaceTargetPos(targetPos);
	m_Player->SetSidewaysRight(false);
}

void TitleScript::LookLeftRight(float dt)
{
	// “見まわし”は、向きだけを左右に振る（移動は0）
	// 例：障害物の中心を見る → 少し左右へオフセット
	const Vector3 base = m_Cover ? m_Cover->GetPosition() : m_Player->GetPosition();

	const float t = m_Timer; // 0..2秒など
	const float x = std::sin(t * 3.0f) * 300.0f;  // 振れ幅（好み）
	Vector3 target = base + Vector3(x, 0.0f, 0.0f);

	FaceTo(target);
}

bool TitleScript::IsEnemyNear(float dist) const
{
	if (!m_Enemy) return true; // 敵いないなら「いる扱い」にして投げる演出でもOK
	Vector3 d = m_Enemy->GetPosition() - m_Player->GetPosition();
	d.y = 0.0f;
	return d.LengthSquared() <= (dist * dist);
}

void TitleScript::EmitRockSound(const Vector3& landPos)
{
	// 石の“着地点”で音を出す（敵AIが SoundManager を見て Investigate する想定）
	WorldSoundEvent ev{};
	ev.Position = landPos;
	ev.Loudness = 1.0f;     // 好みで
	ev.Radius = 1200.0f;  // 好みで
	ev.Type = SoundType::StoneImpact; // 型があるなら

	SoundManager::GetInstance().EmitSound(ev);
}

void TitleScript::StopAndCheckOverWall(void)
{
	m_Player->SetMove(Vector3::Zero, 0.0f);
	m_Player->SetTargetAnim(TitlePlayerActor::TitleAnim::CheckOverWall);
	m_Player->SetSidewaysRight(true);

	// 敵の方向を見る
	if (m_Enemy)
	{
		m_Player->SetFaceMode(TitlePlayerActor::FaceMode::FaceTargetPos);
		m_Player->SetFaceTargetPos(m_Enemy->GetPosition());
	}
}

