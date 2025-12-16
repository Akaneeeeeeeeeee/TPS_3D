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
	m_pObjectManager->Update(deltatime);
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

	// ワールド軸を描画
	SetLineWidth(1.0f);                    // 太さを設定
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}

	Vector3 sp;
	sp = m_Player->GetTransform().GetPosition();
	sp.y -= 500.0f;

	SetLineWidth(3.0f);
	LineDrawerDraw(1000, sp, Vector3(0, 1, 0), Color(1, 1, 0, 1));
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

	// プレイヤー追加
	m_Player = m_pObjectManager->Instantiate<TitlePlayerActor>("PlayerActor", Tag::Player);
	m_Player->SetPosition(Vector3(1920.0f, 148.0f, -3500.0f));

	// スカイドーム
	auto skydome = m_pObjectManager->Instantiate<Skydome>("skydome", Tag::Object);
	skydome->SetTexture("assets/texture/haikei.jpg");

	// 天候オブジェクト
	m_Weather = m_pObjectManager->Instantiate<WeatherController>("WeatherController", Tag::Object);
	m_Weather->SetPosition(Vector3(1920.0f, 750.0f, -3500.0f));

	// 敵追加
	m_Enemy = m_pObjectManager->Instantiate<Enemy>("Enemy", Tag::Enemy);
	m_Enemy->SetTerrain(m_Terrain);

	// 障害物追加
	m_Obstacles[0] = m_pObjectManager->Instantiate<obstacle>("Obstacle1", Tag::Object);
	m_Obstacles[0]->SetPosition(Vector3(-300.0f, 205.0f, 0.0f));
	m_Obstacles[0]->SetScale(Vector3(250.0f, 50.0f, 25.0f));


	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));
}

/**
 * @brief シーンの終了処理
 */
void AnimatedTitleScene::Uninit()
{
	this->ChangeScene = false;
}
