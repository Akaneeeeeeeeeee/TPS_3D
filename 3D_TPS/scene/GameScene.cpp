#include <string>
#include <array>

#include "GameScene.h"
#include "system/debugui.h"
#include "system/AimOrientation.h"
#include "system/SphereDrawer.h"
#include "system/ConeDrawer.h"
#include "system/LineDrawer.h"
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
#include "Framework/Time/Time.h"
#include "Framework/GameObject/StreetLight/StreetLight.h"

namespace {
	// worldTime（0..24） / dayLengthSeconds（現実何秒で1日回すか）/ timeScale（ゲーム内全体倍率）
	static float g_worldTime = 12.0f;          // デフォルトは正午
	static float g_dayLengthSeconds = 60.0f;   // デフォルト：現実60秒で1日まわす（調整しやすい値）
	static float g_timeScale = 1.0f;           // 1.0 = 普通速度、0 = 停止、0.2 = スロー
	static bool  g_manualOverride = false;     // マニュアルで方向/色を指定するか
	static Vector4 g_manualDirection = Vector4(0, 1, 0, 0);
	static Color   g_manualColor = Color(1, 1, 1, 1);

	constexpr float TIME_WARN_SEC = 60.0f;   // ここを閾値にする（例：10秒以下）
	constexpr float TIME_BLINK_SEC = 0.25f;   // 点滅周期（0.25=4回/秒でON/OFF）
	static float g_timeBlinkTimer = 0.0f;
	static bool  g_timeBlinkOn = true;
}

// 現在位置のフィールドの高さ表示
void GameScene::debugFieldHeight() {

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
void GameScene::debugFreeCamera()
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
void GameScene::debugFieldRemake() {

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
void GameScene::debugFieldUnduration() {

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
GameScene::GameScene()
{
}

/**
 * @brief シーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（秒, Time::Deltatime()）
 */
void GameScene::Update(const float deltatime)
{
	if (m_IsGameOver) { return; }

	// ここから通常更新
	m_pObjectManager->Update(deltatime);

	// --- 1) 発見した瞬間にカメラを向ける ---
	if (!m_CameraFocusIssued && m_player)
	{
		for (auto& enemy : m_enemies)
		{
			if (!enemy) continue;

			if (enemy->IsGameOverTriggered())
			{
				m_CameraFocusIssued = true;
				m_FoundByEnemy = enemy;

				m_InFoundSequence = true;
				if (m_pObjectManager && m_pObjectManager->GetGameResult() == ResultType::None)
					m_pObjectManager->SetGameResult(ResultType::Found);

				Vector3 t = enemy->GetTransform().GetPosition();
				t.y += 140.0f; // 敵の頭あたり（調整）
				m_player->StartForceLookAt(t, /*turnSpeed=*/20.0f, /*freezePos=*/true);
				break;
			}
		}
	}

	// --- 2) 射撃モーションが終わったら遷移 ---
	if (m_InFoundSequence)
	{
		if (m_FoundByEnemy && m_FoundByEnemy->IsSceneTransitionRequested())
		{
			SetChangeScene(true);
			SetNextSceneName("ResultScene");
		}
		return; // Found中はここで終わり（制限時間も止まる）
	}
	else
	{
		for (auto& enemy : m_enemies)
		{
			if (!enemy) continue;
			if (enemy->IsSceneTransitionRequested())
			{
				SetChangeScene(true);
				SetNextSceneName("ResultScene");
				return;
			}
		}
	}

	// Found 演出中は制限時間を止め、時間切れ/クリア判定もしない
	if (m_InFoundSequence) { return; }

	// タイマー更新（dt=0 なら止まる）
	m_Limit.Update(deltatime);

	// ゴール到達判定（クリア優先）
	if (m_Goal->IsReached())
	{
		if (m_pObjectManager) m_pObjectManager->SetGameResult(ResultType::Clear);

		SetChangeScene(true);
		SetNextSceneName("ResultScene");
		return;
	}

	// 時間切れ
	if (m_Limit.IsTimeout())
	{
		// クリアが既に確定してるなら上書きしない（同フレーム対策）
		if (m_pObjectManager && m_pObjectManager->GetGameResult() != ResultType::Clear)
			m_pObjectManager->SetGameResult(ResultType::TimeUp);

		m_IsGameOver = true;
		SetChangeScene(true);
		SetNextSceneName("ResultScene");
		return;
	}

	// 敵再配置リクエストが来ていたら実行
	if (m_RequestRebuildEnemies)
	{
		m_RequestRebuildEnemies = false;
		RebuildEnemies();
	}
}


/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void GameScene::Draw(void)
{
	// オブジェクト描画
	m_pObjectManager->Draw();

	// ワールド軸を描画
	// 3軸カラー
	Color axiscol[3] = {
		Color(1, 0, 0, 1),
		Color(0, 1, 0, 1),
		Color(0, 1, 1, 1)
	};
	// 太さを設定
	SetLineWidth(1.0f);
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}

	Vector3 sp;
	sp = m_player->GetTransform().GetPosition();
	sp.y -= 500.0f;

	SetLineWidth(3.0f);
	//LineDrawerDraw(1000, sp, Vector3(0, 1, 0), Color(1, 1, 0, 1));

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
}

/**
 * @brief シーンの初期化処理
 */
void GameScene::Init(ObjectManager* mgr)
{
	m_pObjectManager = mgr;
	// 前回のゲーム結果をクリア
	if (m_pObjectManager) m_pObjectManager->ClearGameResult();
	m_IsGameOver = false;
	m_InFoundSequence = false;
	m_CameraFocusIssued = false;
	m_FoundByEnemy = nullptr;
	m_RequestRebuildEnemies = false;
	m_Limit.Start(120.0f);

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	m_playersegment[0] = std::make_unique<Segment>(Vector3(0, -100, 0), Vector3(0, 100, 0));

	// フィールド初期化
	m_terrain = m_pObjectManager->Instantiate<Terrain>("city", Tag::Field);
	m_terrain->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	m_terrain->SetScale(Vector3(100.0f, 100.0f, 100.0f));
	m_terrain->SetScene(this);

	// プレイヤ
	m_player = m_pObjectManager->Instantiate<Player>("player", Tag::Player);
	m_player->SetPosition(Vector3(-300.0f, 100.0f, -100.0f));
	//m_player->SetPosition(Vector3(0.0f, 10.0f, -200.0f));

	// 天候オブジェクト
	auto weather = m_pObjectManager->Instantiate<WeatherController>("WeatherController", Tag::Object);
	weather->SetPosition(Vector3(0.0f, 500.0f, 0.0f));
	// ゴール
	m_Goal = m_pObjectManager->Instantiate<Goal>("goal", Tag::Goal);
	m_Goal->SetScale(Vector3(0.5f, 1.0f, 0.5f));
	m_Goal->SetPosition(Vector3(-300.0f, 0.0f, -800.0f));


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
		//auto obstacleObj = m_pObjectManager->Instantiate<obstacle>("Obstacle" + std::to_string(0), Tag::Object, this);

		// 落下テスト用
		//obstacleObj->SetPosition(Vector3(-300.0f, 500.0f, -100.0f));
		//obstacleObj->SetScale(Vector3(25.0f, 25.0f, 25.0f));
		//obstacleObj->SetPosition(Vector3(-300.0f, 205.0f, 0.0f));
		//obstacleObj->SetScale(Vector3(250.0f, 50.0f, 25.0f));
		//m_obstacles[0] = obstacleObj;

		// 敵
		SpawnEnemies(m_MultiEnemy ? m_MultiCount : 1);

		// 街灯
		auto streetLight = m_pObjectManager->Instantiate<StreetLight>("StreetLight1", Tag::Light);
		//streetLight->SetNightOnly(false);
		//streetLight->SetEnabled(true);
		//streetLight->SetIntensity(200.0f); // 目視確認用に極端に上げる
		streetLight->SetPosition(Vector3(-300.0f, 400.0f, -100.0f));
		// 地面の明るい円半径を直接指定
		streetLight->SetGroundCircle(
			/*groundRadius=*/150.0f,
			/*groundY=*/0.0f,
			/*topRadiusMin=*/10.0f,   // 上面の“口径”を確保
			/*innerRatio=*/0.6f       // 中心が強い範囲
		);
	}

	// DirectWrite 初期化（1回だけ）
	if (!m_pDirectWrite)
	{
		m_FontData.fontSize = 24.0f;
		m_FontData.Color = D2D1::ColorF(D2D1::ColorF::White);
		m_FontData.shadowOffset = D2D1::Point2F(2.0f, -2.0f);

		m_pDirectWrite = std::make_unique<DirectWrite>(&m_FontData);

		// Renderer が管理している swapchain を渡す（ここがズレると表示されない）
		HRESULT hr = m_pDirectWrite->Init(Renderer::GetSwapChain());
		assert(SUCCEEDED(hr));
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
void GameScene::Uninit()
{
	Time::GetInstance().SetTimeScale(1.0f);
}

void GameScene::ClearEnemies()
{
	for (int i = 0; i < m_EnemyAliveCount; ++i)
	{
		if (!m_enemies[i]) continue;
		m_pObjectManager->DeleteObject("Enemy_" + std::to_string(i));
		m_enemies[i] = nullptr;
	}
	m_EnemyAliveCount = 0;
}

bool GameScene::MakeRandomSpawnPos(Vector3& outPos, const std::vector<Vector3>& used)
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
	if (!m_TerrainCol->GetWorldXZBounds(xzMin, xzMax)) return false;

	auto rng = RandomEngine::tls().stream("EnemySpawn");

	constexpr int   MAX_TRY = 32;
	constexpr float HEIGHT_OFFSET = 5.0f;
	constexpr float MIN_SEP = 200.0f; // 敵同士の最低距離

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

void GameScene::SpawnEnemies(int count)
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
				// ランダム失敗時の保険：プレイヤー近く等に置かないなら別条件も足す
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

void GameScene::RebuildEnemies()
{
	ClearEnemies();
	int spawnCount = m_MultiEnemy ? m_MultiCount : 1;
	SpawnEnemies(spawnCount);
}

void GameScene::DrawUI(void)
{
	if (!m_pDirectWrite) return;

	// 目標テキストはワイド文字リテラルを使用
	const wchar_t* objectiveTextW = L"目標：敵に見つからずにゴールを目指せ！";
	m_pDirectWrite->DrawString(
		objectiveTextW,
		{ 20.0f, 20.0f }, D2D1_DRAW_TEXT_OPTIONS_NONE, true);

	// 残り秒 → mm:ss
	int s = (m_Limit.remain > 0.0f) ? (int)std::ceil(m_Limit.remain) : 0;
	int mm = s / 60;
	int ss = s % 60;

	wchar_t timeBuf[64];
	swprintf_s(timeBuf, L"TIME %02d:%02d", mm, ss);

	// 色を一時変更して TIME だけ描く
	if (m_Limit.remain <= TIME_WARN_SEC)
	{
		g_timeBlinkTimer += Time::GetInstance().Deltatime();
		if (g_timeBlinkTimer >= TIME_BLINK_SEC)
		{
			g_timeBlinkTimer = 0.0f;
			g_timeBlinkOn = !g_timeBlinkOn;
		}

		const float alpha = g_timeBlinkOn ? 1.0f : 0.15f;
		m_pDirectWrite->SetTextColor(D2D1::ColorF(D2D1::ColorF::Red, alpha));
	}
	else
	{
		g_timeBlinkTimer = 0.0f;
		g_timeBlinkOn = true;
		m_pDirectWrite->SetTextColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
	}

	m_pDirectWrite->DrawString(timeBuf, { 20.0f, 100.0f }, D2D1_DRAW_TEXT_OPTIONS_NONE, true);

	// 念のため通常色に戻す（他の文字に影響させない）
	m_pDirectWrite->SetTextColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
#if _DEBUG
	// 既存の表示
	if (m_player)
	{
		const auto p = m_player->GetTransform().GetPosition();
		wchar_t buf[256];
		swprintf_s(buf, L"Player (%.1f, %.1f, %.1f)", p.x, p.y, p.z);
		m_pDirectWrite->DrawString(buf, { 20.0f, 50.0f }, D2D1_DRAW_TEXT_OPTIONS_NONE, true);
	}
#endif
}