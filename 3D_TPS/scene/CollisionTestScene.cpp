#include <string>
#include <array>

#include "CollisionTestScene.h"
#include "system/debugui.h"
#include "system/AimOrientation.h"
#include "system/SphereDrawer.h"
#include "system/ConeDrawer.h"
#include "system/LineDrawer.h"
#include "GameObject/Skydome.h"
#include "GameObject/Rock.h"
#include "Framework/GameObject/Goal/Goal.h"
#include "Framework/GameObject/Terrain/Terrain.h"
#include "Framework/GameObject/WeatherController/WeatherController.h"

#include "system/TriangleDrawer.h"
#include "system/meshmanager.h"
#include "system/RandomEngine.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "Framework/Component/Physic/BoxCollider.h"
#include "Framework/Component/Physic/CapsuleCollider.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "commontypes.h"

namespace {
	// worldTime（0..24） / dayLengthSeconds（現実何秒で1日回すか）/ timeScale（ゲーム内全体倍率）
	static float g_worldTime = 12.0f;          // デフォルトは正午
	static float g_dayLengthSeconds = 60.0f;   // デフォルト：現実60秒で1日まわす（調整しやすい値）
	static float g_timeScale = 1.0f;           // 1.0 = 普通速度、0 = 停止、0.2 = スロー
	static bool  g_manualOverride = false;     // マニュアルで方向/色を指定するか
	static Vector4 g_manualDirection = Vector4(0, 1, 0, 0);
	static Color   g_manualColor = Color(1, 1, 1, 1);
}

// --- 以下は元の関数群。ほとんどそのまま。 --- 

// 現在位置のフィールドの高さ表示
void CollisionTestScene::debugFieldHeight() {

	//ImGui::Begin("debug Field Height");

	//Transform& srt = m_player->TransformRef();

	//ImGui::SliderFloat3("player pos ", &srt.PositionRef().x, -100, 100);

	//int sqno = m_field->GetSquareNo(srt.GetPosition());
	//ImGui::SliderInt("square no ", &sqno, -100, 100);

	//std::array<Field::Face, 2> retfaces;
	//if (sqno != -1) {
	//	m_field->GetFace(srt.GetPosition(), retfaces);
	//}
	//ImGui::SliderInt3("Face index1 ", &retfaces[0].idx[0], -100, 100);
	//ImGui::SliderInt3("Face index2 ", &retfaces[1].idx[0], -100, 100);

	//std::array<Vector3, 3> vertices1;
	//std::array<Vector3, 3> vertices2;

	//if (sqno != -1) {
	//	m_field->GetFaceVertex(sqno * 2, vertices1);
	//	m_field->GetFaceVertex(sqno * 2 + 1, vertices2);
	//}

	//ImGui::Separator();
	//ImGui::SliderFloat3("Face1 Vertex1 ", &vertices1[0].x, -500, 500);
	//ImGui::SliderFloat3("Face1 Vertex2 ", &vertices1[1].x, -500, 500);
	//ImGui::SliderFloat3("Face1 Vertex3 ", &vertices1[2].x, -500, 500);

	//ImGui::Separator();
	//ImGui::SliderFloat3("Face2 Vertex1 ", &vertices2[0].x, -500, 500);
	//ImGui::SliderFloat3("Face2 Vertex2 ", &vertices2[1].x, -500, 500);
	//ImGui::SliderFloat3("Face2 Vertex3 ", &vertices2[2].x, -500, 500);

	//ImGui::End();
}


// デバッグフリーカメラ
void CollisionTestScene::debugFreeCamera()
{
	ImGui::Begin("debug Free camera");

	static float radius = 800.0f;
	static Vector3 pos = Vector3(0, 0, radius);
	static Vector3 lookat = Vector3(0, 0, 0);
	static float elevation = -90.0f * PI / 180.0f;
	static float azimuth = PI / 2.0f;

	static Vector3 spherecenter = Vector3(0, 0, 0);

	ImGui::SliderFloat("Radius", &radius, 1, 1500);
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

// フィールド再作成
void CollisionTestScene::debugFieldRemake() {

	ImGui::Begin("debug Field Remake");

	static int dividex = 50;
	static int dividez = 50;

	static float width = 2500.0f;
	static float depth = 2500.0f;

	ImGui::SliderFloat("width", &width, 10, 1000);
	ImGui::SliderFloat("depth", &depth, 10, 1000);
	ImGui::SliderInt("devide x", &dividex, 1, 200);
	ImGui::SliderInt("devide z", &dividez, 1, 200);

	if (ImGui::Button("recreate  field")) {

		// フィールド初期化
		m_pObjectManager->DeleteObject("field");
		m_field = m_pObjectManager->Instantiate<Field>("field", Tag::Field);
		m_field->setdepth(depth);
		m_field->setwidth(width);
		m_field->setdividex(dividex);
		m_field->setdividez(dividez);
	}

	ImGui::End();
}

// フィールドに凸凹にする
void CollisionTestScene::debugFieldUnduration() {

	ImGui::Begin("debug Field Remake with unduration");

	static float minheight = 0.0f;
	static float maxheight = 100.0f;

	ImGui::SliderFloat("low height", &minheight, 0.0f, 10.0f);
	ImGui::SliderFloat("max hight", &maxheight, 0.0f, 100.0f);

	static float perlinscale = 0.5f;     // ノイズの細かさ（お好みで 0.02～0.2 くらい）
	static float perlinoffsetX = 10.0f;   // シード代わりのオフセット（任意）
	static float perlinoffsetZ = 10.0f;

	ImGui::SliderFloat("perlin scale", &perlinscale, 0.0f, 5.0f);
	ImGui::SliderFloat("perlin offset x", &perlinoffsetX, 10.0f, 800.0f);
	ImGui::SliderFloat("perlin offset z", &perlinoffsetZ, 10.0f, 800.0f);

	if (ImGui::Button("remake field with random")) {
		m_field->makeundulationwithrandom(minheight, minheight + maxheight);
	}
	if (ImGui::Button("remake field with perlin")) {

		m_field->makeundulationwithperlin(minheight, minheight + maxheight,
			perlinscale, perlinoffsetX, perlinoffsetZ);
	}

	ImGui::End();

}

/**
 * @brief コンストラクタ
 */
CollisionTestScene::CollisionTestScene()
{
}

/**
 * @brief シーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（秒, Time::Deltatime()）
 */
void CollisionTestScene::Update(const float deltatime)
{
	m_pObjectManager->Update(deltatime);

	if (m_RequestRebuildEnemies)
	{
		m_RequestRebuildEnemies = false;
		RebuildEnemies();
	}

	if (m_Goal->IsReached())
	{
		SetChangeScene(true);
		SetNextSceneName("ResultScene");
	}

	for(auto& enemy : m_enemies) 
	{
		if (enemy && enemy->IsGameOverTriggered())
		{
			SetChangeScene(true);
			SetNextSceneName("ResultScene");
			break;
		}
	}
}


/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void CollisionTestScene::Draw(void)
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
	sp = m_player->GetTransform().GetPosition();
	sp.y -= 500.0f;

	SetLineWidth(3.0f);
	LineDrawerDraw(1000, sp, Vector3(0, 1, 0), Color(1, 1, 0, 1));

	//int sqno = m_field->GetSquareNo(m_player->GetTransform().GetPosition());

	//std::array<Field::Face, 2> retfaces;
	//std::array<Vector3, 3> vertices1;
	//std::array<Vector3, 3> vertices2;

	//if (sqno != -1) {
	//	Vector3 pos = m_player->GetTransform().GetPosition();

	//	m_field->GetFace(pos, retfaces);
	//	m_field->GetFaceVertex(sqno * 2, vertices1);
	//	m_field->GetFaceVertex(sqno * 2 + 1, vertices2);

	//	TriangleDrawerDraw(vertices1, Color(1, 0, 0, 1));
	//	TriangleDrawerDraw(vertices2, Color(1, 1, 0, 1));
	//}

	m_pObjectManager->Draw();
}

/**
 * @brief シーンの初期化処理
 */
void CollisionTestScene::Init(ObjectManager* mgr)
{
	m_pObjectManager = mgr;

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	m_playersegment[0] = std::make_unique<Segment>(Vector3(0, -100, 0), Vector3(0, 100, 0));


	// フィールド初期化
	//m_field = m_pObjectManager->Instantiate<Field>("field", Tag::Field);
	//m_field->SetPosition(Vector3(0.0f, -100.0f, 0.0f));
	m_terrain = m_pObjectManager->Instantiate<Terrain>("city", Tag::Field);
	m_terrain->SetPosition(Vector3(0.0f, 100.0f, 0.0f));
	m_terrain->SetScale(Vector3(100.0f, 100.0f, 100.0f));


	// プレイヤ
	m_player = m_pObjectManager->Instantiate<Player>("player", Tag::Player);
	m_player->SetPosition(Vector3(-300.0f, 210.0f, -100.0f));
	//m_player->SetPosition(Vector3(0.0f, 10.0f, -200.0f));

	// スカイドーム
	auto skydome = m_pObjectManager->Instantiate<Skydome>("skydome", Tag::Object);
	skydome->SetTexture("assets/texture/haikei.jpg");

	// 天候オブジェクト
	auto weather = m_pObjectManager->Instantiate<WeatherController>("WeatherController", Tag::Object);
	weather->SetPosition(Vector3(0.0f, 500.0f, 0.0f));
	// ゴール
	m_Goal = m_pObjectManager->Instantiate<Goal>("goal", Tag::Goal);
	m_Goal->SetScale(Vector3(50.0f, 100.0f, 50.0f));
	m_Goal->SetPosition(Vector3(-300.0f, 250.0f, -800.0f));


	// --- 衝突テスト用障害物 ---
	{
		// 大きい床（Static）
		{
			//m_field = m_pObjectManager->Instantiate<Field>("Field", Tag::Object);

			//Transform tf = m_field->GetTransform();
			//tf.SetScale(Vector3(500.0f, 1.0f, 500.0f));   // 横長・薄い床
			//tf.SetPosition(Vector3(0.0f, -50.0f, 0.0f));   // 少し下げて床位置へ
			//tf.SetRotation(Quaternion::Identity);

			//m_field->SetTransform(tf);

			// Rigidbody を Static に
			/*ground->AddComponent<BoxCollider>("boxcollider")->Init();
			auto rb = ground->AddComponent<Rigidbody>("Rigidbody", 1.0f);
			rb->SetBodyType(Rigidbody::Type::Static);
			rb->Init();*/
		}
		
		// 障害物
		auto obstacleObj = m_pObjectManager->Instantiate<obstacle>("Obstacle" + std::to_string(0), Tag::Object, this);
		// 落下テスト用
		//obstacleObj->SetPosition(Vector3(-300.0f, 500.0f, -100.0f));
		//obstacleObj->SetScale(Vector3(25.0f, 25.0f, 25.0f));
		obstacleObj->SetPosition(Vector3(-300.0f, 205.0f, 0.0f));
		obstacleObj->SetScale(Vector3(250.0f, 50.0f, 25.0f));
		m_obstacles[0] = obstacleObj;

		// 敵
		//auto enemyObj = m_pObjectManager->Instantiate<Enemy>("Enemy_" + std::to_string(0), Tag::Enemy);
		//enemyObj->SetPosition(Vector3(-300.0f, 210.0f, 750.0f));
		//enemyObj->SetPlayer(m_player);
		//enemyObj->SetTerrain(m_terrain);
		//// 配列に保持
		//m_enemies[0] = enemyObj;

		SpawnEnemies(m_MultiEnemy ? m_MultiCount : 1);
	}

	// デバッグ Free Camera
	DebugUI::RedistDebugFunction([this]() {
		debugFreeCamera();
		});

	// remake field
	DebugUI::RedistDebugFunction([this]() {
		debugFieldRemake();
		});

	// remake undulation
	DebugUI::RedistDebugFunction([this]() {
		debugFieldUnduration();
		});

	DebugUI::RedistDebugFunction([this]() {
		debugFieldHeight();
		});

	TriangleDrawerInit();
}

/**
 * @brief シーンの終了処理
 */
void CollisionTestScene::Uninit()
{
}

void CollisionTestScene::ClearEnemies()
{
	for (int i = 0; i < m_EnemyAliveCount; ++i)
	{
		if (!m_enemies[i]) continue;
		m_pObjectManager->DeleteObject("Enemy_" + std::to_string(i));
		m_enemies[i] = nullptr;
	}
	m_EnemyAliveCount = 0;
}

bool CollisionTestScene::MakeRandomSpawnPos(Vector3& outPos, const std::vector<Vector3>& used)
{
	// 地形コライダ取得
	if (!m_TerrainCol)
	{
		auto terrainCol = m_terrain->GetComponent<StaticMeshCollider>();
		if (terrainCol)
		{
			m_TerrainCol = terrainCol;
		}
		else
		{
			return false;
		}
	}

	Vector3 xzMin, xzMax;
	if (!m_TerrainCol->GetWorldXZBounds(xzMin, xzMax)) return false; // 既存前提（なければ固定範囲で代用）

	auto rng = RandomEngine::tls().stream("EnemySpawn");

	constexpr int   MAX_TRY = 32;
	constexpr float HEIGHT_OFFSET = 5.0f;
	constexpr float MIN_SEP = 200.0f; // 敵同士の最低距離（好み）

	for (int t = 0; t < MAX_TRY; ++t)
	{
		float x = static_cast<float>(rng.uniformReal(xzMin.x, xzMax.x));
		float z = static_cast<float>(rng.uniformReal(xzMin.z, xzMax.z));

		float y;
		if (!m_TerrainCol->SampleHeight(x, z, y)) continue;

		Vector3 p(x, y + HEIGHT_OFFSET, z);

		bool ok = true;
		for (auto& u : used)
		{
			if ((p - u).LengthSquared() < (MIN_SEP * MIN_SEP)) { ok = false; break; }
		}
		if (!ok) continue;

		outPos = p;
		return true;
	}
	return false;
}

void CollisionTestScene::SpawnEnemies(int count)
{
	count = std::clamp(count, 1, static_cast<int>(ENEMYMAX));

	std::vector<Vector3> used;
	used.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		auto enemyObj = m_pObjectManager->Instantiate<Enemy>(
			"Enemy_" + std::to_string(i), Tag::Enemy);

		Vector3 spawnPos = m_SingleEnemyPos;

		if (count == 1)
		{
			// 1体モード：元の座標
			spawnPos = m_SingleEnemyPos;
		}
		else
		{
			// 複数モード：ランダム
			if (!MakeRandomSpawnPos(spawnPos, used))
			{
				// ランダム失敗時の保険（推測です）：プレイヤー近く等に置かないなら別条件も足す
				spawnPos = m_SingleEnemyPos + Vector3(0, 0, 300.0f * i);
			}
		}

		enemyObj->SetPosition(spawnPos);
		enemyObj->SetPlayer(m_player);
		enemyObj->SetTerrain(m_terrain);

		used.push_back(spawnPos);
		m_enemies[i] = enemyObj;
	}

	m_EnemyAliveCount = count;
}

void CollisionTestScene::RebuildEnemies()
{
	ClearEnemies();
	int spawnCount = m_MultiEnemy ? m_MultiCount : 1;
	SpawnEnemies(spawnCount);
}