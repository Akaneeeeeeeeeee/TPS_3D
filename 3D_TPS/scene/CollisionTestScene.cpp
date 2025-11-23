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

#include "system/TriangleDrawer.h"
#include "system/meshmanager.h"
#include "system/RandomEngine.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "Framework/Component/Physic/BoxCollider.h"
#include "Framework/Component/Physic/CapsuleCollider.h"
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

	ImGui::Begin("debug Field Height");

	Transform& srt = m_player->TransformRef();

	ImGui::SliderFloat3("player pos ", &srt.PositionRef().x, -100, 100);

	int sqno = m_field->GetSquareNo(srt.GetPosition());
	ImGui::SliderInt("square no ", &sqno, -100, 100);

	std::array<Terrain::Face, 2> retfaces;
	if (sqno != -1) {
		m_field->GetFace(srt.GetPosition(), retfaces);
	}
	ImGui::SliderInt3("Face index1 ", &retfaces[0].idx[0], -100, 100);
	ImGui::SliderInt3("Face index2 ", &retfaces[1].idx[0], -100, 100);

	std::array<Vector3, 3> vertices1;
	std::array<Vector3, 3> vertices2;

	if (sqno != -1) {
		m_field->GetFaceVertex(sqno * 2, vertices1);
		m_field->GetFaceVertex(sqno * 2 + 1, vertices2);
	}

	ImGui::Separator();
	ImGui::SliderFloat3("Face1 Vertex1 ", &vertices1[0].x, -500, 500);
	ImGui::SliderFloat3("Face1 Vertex2 ", &vertices1[1].x, -500, 500);
	ImGui::SliderFloat3("Face1 Vertex3 ", &vertices1[2].x, -500, 500);

	ImGui::Separator();
	ImGui::SliderFloat3("Face2 Vertex1 ", &vertices2[0].x, -500, 500);
	ImGui::SliderFloat3("Face2 Vertex2 ", &vertices2[1].x, -500, 500);
	ImGui::SliderFloat3("Face2 Vertex3 ", &vertices2[2].x, -500, 500);

	ImGui::End();
}

// 平行光源の方向セット（デバッグ + 時刻/色操作UIをここに統合）
void CollisionTestScene::debugDirectionalLight()
{
	ImGui::Begin("debug Directional Light");

	// World time controls
	ImGui::Text("World Time (hours): %.3f", g_worldTime);
	if (ImGui::SliderFloat("WorldTime", &g_worldTime, 0.0f, 24.0f)) {
		// clamp
		if (g_worldTime < 0.0f) g_worldTime = 0.0f;
		if (g_worldTime >= 24.0f) g_worldTime = fmodf(g_worldTime, 24.0f);
	}

	// day length and timescale
	ImGui::SliderFloat("Day length (sec)", &g_dayLengthSeconds, 1.0f, 3600.0f); // 1秒～1時間
	ImGui::SliderFloat("TimeScale (global)", &g_timeScale, 0.0f, 5.0f);

	// manual override switch
	ImGui::Checkbox("Manual override (direction/color)", &g_manualOverride);
	if (g_manualOverride) {
		ImGui::SliderFloat3("Manual Direction", &g_manualDirection.x, -1.0f, 1.0f);
		Vector3 tmpDir(g_manualDirection.x, g_manualDirection.y, g_manualDirection.z);
		if (tmpDir.Length() < 1e-6f) tmpDir = Vector3(0, 1, 0);
		tmpDir.Normalize();
		g_manualDirection = Vector4(tmpDir.x, tmpDir.y, tmpDir.z, 0.0f);

		float col[4] = { g_manualColor.R(), g_manualColor.G(), g_manualColor.B(), g_manualColor.A() };
		if (ImGui::ColorEdit4("Manual Color", col)) {
			g_manualColor = Color(col[0], col[1], col[2], col[3]);
		}
	}

	// show computed values (for debug)
	// Compute current sun direction & color for display (without setting renderer yet)
	// Use same logic as LightUpdate but without consuming dt.
	// angle from current worldTime:
	float angle = (g_worldTime / 24.0f) * DirectX::XM_2PI;
	Vector4 computedDir(0.0f, sinf(angle), cosf(angle), 0.0f);
	computedDir.Normalize();
	Color computedColor = GetSunColor(g_worldTime);
	float intensity = std::max(0.0f, computedDir.y);

	ImGui::Text("Computed Direction: %.3f, %.3f, %.3f", computedDir.x, computedDir.y, computedDir.z);
	ImGui::Text("Computed Color:  r:%.3f g:%.3f b:%.3f  intensity: %.3f",
		computedColor.R(), computedColor.G(), computedColor.B(), intensity);

	ImGui::End();
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
		m_field = m_pObjectManager->Instantiate<Terrain>("field", Tag::Field);
		m_field->setdepth(depth);
		m_field->setwidth(width);
		m_field->setdividex(dividex);
		m_field->setdividez(dividez);

		m_field->Init();
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
	// deltatime は秒（Time::Instance().Deltatime()から渡される）
	LightUpdate(deltatime);

	// プレイヤー更新
	//m_player->Update(deltatime);

	// 現在位置取得
	Vector3 pos = m_player->GetTransform().GetPosition();

	// 地形に合わせて高さ修正
	float height = m_field->GetHeight(pos);
	pos.y = height;

	// 修正した位置を反映
	//m_player->SetPosition(pos);


	// 障害物
	/*for (auto& obs : m_obstacles) {

		Vector3 pos = obs->GetTransform().GetPosition();
		float height = m_field->GetHeight2(pos);
		pos.y = height;

		obs->SetPosition(pos);
	}*/

	m_pObjectManager->Update(deltatime);
}

// ---------- 色補間 ----------
Color CollisionTestScene::LerpColor(const Color& a, const Color& b, float t)
{
	return a + (b - a) * t;
}

// ---------- 時間帯に応じた太陽色 ----------
Color CollisionTestScene::GetSunColor(float t)   // t：worldTime(0?24)
{
	// 夜
	const Color nightColor = Color(0.05f, 0.07f, 0.2f);

	// 朝焼け
	const Color morningColor = Color(1.0f, 0.4f, 0.2f);

	// 昼
	const Color noonColor = Color(1.0f, 1.0f, 1.0f);

	// 夕焼け
	const Color sunsetColor = Color(1.0f, 0.5f, 0.2f);

	// 夜 → 朝（0?4）
	if (t < 4.0f) {
		return nightColor;
	}
	// 朝焼け（4?7）
	else if (t < 7.0f) {
		float k = (t - 4.0f) / 3.0f;
		return LerpColor(nightColor, morningColor, k);
	}
	// 朝焼け → 昼（7?16）
	else if (t < 16.0f) {
		float k = (t - 7.0f) / 9.0f;
		return LerpColor(morningColor, noonColor, k);
	}
	// 昼 → 夕焼け（16?19）
	else if (t < 19.0f) {
		float k = (t - 16.0f) / 3.0f;
		return LerpColor(noonColor, sunsetColor, k);
	}
	// 夕焼け → 夜（19?24）
	else {
		float k = (t - 19.0f) / 5.0f;
		return LerpColor(sunsetColor, nightColor, k);
	}
}

void CollisionTestScene::LightUpdate(float dt)
{
	// dt は秒単位（既に global Time::DeltaTime が適用済み）
	// g_dayLengthSeconds が 0 以下の場合は 1 に補正
	if (g_dayLengthSeconds <= 0.0f) g_dayLengthSeconds = 1.0f;

	// UI から操作可能なローカル時間スケールを適用
	float scaledDt = dt * g_timeScale;

	// 世界時間を更新（1 日が g_dayLengthSeconds 秒で 24 時間進む）
	g_worldTime += (scaledDt / g_dayLengthSeconds) * 24.0f;

	// 世界時間を 0?24 に正規化
	if (g_worldTime >= 24.0f) g_worldTime = fmodf(g_worldTime, 24.0f);
	if (g_worldTime < 0.0f) g_worldTime += 24.0f;

	// 世界時間 → 太陽角度（0?24 時間 → 0?2π）
	float sunAngle = ((g_worldTime / 24.0f) * DirectX::XM_2PI) - DirectX::XM_PIDIV2; // π/2 ずらす

	// 太陽の方向ベクトルを計算
	Vector4 sunDir;
	if (g_manualOverride)
	{
		// マニュアルオーバーライド時は指定の方向を使用（正規化済み）
		sunDir = g_manualDirection;
	}
	else
	{
		sunDir.x = 0.0f;
		sunDir.y = sinf(sunAngle);  // 高さ
		sunDir.z = cosf(sunAngle);  // 前後方向
		sunDir.w = 0.0f;
		sunDir.Normalize();
	}

	// 太陽の色を取得
	Color sunColor = g_manualOverride ? g_manualColor : GetSunColor(g_worldTime);

	// 太陽の高さに応じて光の強度を調整（y が負なら夜として 0 に）
	//float intensity = std::max(0.0f, sunDir.y);
	// 太陽高度補正
	float heightFactor = std::clamp(sunDir.y, 0.0f, 1.0f);
	float intensity = pow(heightFactor, 0.5f);   // 非線形補正

	// ライト構造体に設定
	//LIGHT light{};
	//light.Enable = true;
	//light.Direction = sunDir;
	//light.Diffuse = sunColor * intensity;
	// 昼はアンビエントも増加
	//light.Ambient = Color(0.2f, 0.2f, 0.2f) * (0.5f + intensity * 0.5f);
	// sunDir.y が低いときも夜を真っ暗にせず、環境色を追加
	/*Color ambient = LerpColor(Color(0.05f, 0.07f, 0.2f), sunColor, intensity * 0.5f);
	light.Ambient = ambient;*/

	// --- 簡易GI（環境光） ---
   // スカイからの光
	Color skyColor = Color(0.3f, 0.4f, 0.5f) * (0.5f + 0.5f * intensity);

	// 地面反射光（地面が明るい色ならより明るくなる）
	Color groundColor = Color(0.1f, 0.1f, 0.05f) * (1.0f - intensity);

	// 合成（太陽＋スカイ＋地面）
	Color totalDiffuse = sunColor * intensity + skyColor + groundColor;

	// ライト構造体に設定
	LIGHT light{};
	light.Enable = true;
	light.Direction = sunDir;
	light.Diffuse = totalDiffuse;

	// アンビエントもGIっぽく増加
	light.Ambient = Color(0.2f, 0.2f, 0.2f) * (0.5f + 0.5f * intensity) + groundColor * 0.2f;

	Renderer::SetLight(light);

	// レンダラーにライトをセット
	//Renderer::SetLight(light);
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void CollisionTestScene::Draw(void)
{
	m_camera.Draw();

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

	int sqno = m_field->GetSquareNo(m_player->GetTransform().GetPosition());

	std::array<Terrain::Face, 2> retfaces;
	std::array<Vector3, 3> vertices1;
	std::array<Vector3, 3> vertices2;

	if (sqno != -1) {
		Vector3 pos = m_player->GetTransform().GetPosition();

		m_field->GetFace(pos, retfaces);
		m_field->GetFaceVertex(sqno * 2, vertices1);
		m_field->GetFaceVertex(sqno * 2 + 1, vertices2);

		TriangleDrawerDraw(vertices1, Color(1, 0, 0, 1));
		TriangleDrawerDraw(vertices2, Color(1, 1, 0, 1));
	}

	m_pObjectManager->Draw();
}

/**
 * @brief シーンの初期化処理
 */
void CollisionTestScene::Init(ObjectManager* mgr)
{
	m_pObjectManager = mgr;
	// カメラ(3D)の初期化
	m_camera.Init();

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	m_playersegment[0] = std::make_unique<Segment>(Vector3(0, -100, 0), Vector3(0, 100, 0));

	// 光源計算なしシェーダー
	std::unique_ptr<CShader> shader = std::make_unique<CShader>();
	shader->Create("shader/vertexLightingVS.hlsl", "shader/vertexLightingPS.hlsl");
	MeshManager::RegisterShader<CShader>("unlightshader", std::move(shader));

	// メッシュデータ読み込み（障害物用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/obj/box.obj", "assets/model/obj/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("obstaclebox", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("obstaclebox", std::move(srenderer));

		// 岩用
		CStaticMesh* rockmesh = AssetManager::GetInstance().GetStaticMesh("Rock");
		std::unique_ptr<CStaticMeshRenderer> rockrenderer = std::make_unique<CStaticMeshRenderer>();
		rockrenderer->Init(*rockmesh);
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("obstaclerock", std::move(rockrenderer));
	}

	// フィールド初期化
	m_field = m_pObjectManager->Instantiate<Terrain>("field", Tag::Field);

	// プレイヤ
	m_player = m_pObjectManager->Instantiate<Player>("player", Tag::Player);
	//m_player->SetPosition(Vector3(0.0f, 100.0f, 0.0f));
	m_player->SetPosition(Vector3(0.0f, 10.0f, -200.0f));
	m_player->SetCamera(&m_camera);

	// スカイドーム
	auto skydome = m_pObjectManager->Instantiate<Skydome>("skydome", Tag::Object);
	skydome->SetTexture("assets/texture/haikei.jpg");

	// 岩
	/*auto rock = m_pObjectManager->Instantiate<Rock>("obstacleRock1", Tag::Object);
	rock->Init();*/


	// --- 衝突テスト用障害物 ---
	{
		// 大きい床（Static）
		{
			auto ground = m_pObjectManager->Instantiate<Terrain>("Terrain", Tag::Object);

			Transform tf = ground->GetTransform();
			tf.SetScale(Vector3(500.0f, 1.0f, 500.0f));   // 横長・薄い床
			tf.SetPosition(Vector3(0.0f, -50.0f, 0.0f));   // 少し下げて床位置へ
			tf.SetRotation(Quaternion::Identity);

			ground->SetTransform(tf);

			// Rigidbody を Static に
			/*ground->AddComponent<BoxCollider>("boxcollider")->Init();
			auto rb = ground->AddComponent<Rigidbody>("Rigidbody", 1.0f);
			rb->SetBodyType(Rigidbody::Type::Static);
			rb->Init();*/
		}

		auto& rng = RandomEngine::tls();
		rng.uniformReal(-500, 500);

		// 障害物
		// Instantiate
		auto obstacleObj = m_pObjectManager->Instantiate<obstacle>("Obstacle" + std::to_string(0), Tag::Object, this);
		obstacleObj->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
		obstacleObj->SetScale(Vector3(50.0f, 50.0f, 300.0f));
		m_obstacles[0] = obstacleObj;

		//for (int cnt = 0; cnt < OBSTACLEMAX; cnt++)
		//{
		//	// Instantiate
		//	auto obstacleObj =
		//		m_pObjectManager->Instantiate<obstacle>(
		//			"Obstacle" + std::to_string(cnt),
		//			Tag::Object,
		//			this
		//		);

		//	// Transform を取得
		//	Transform tf = obstacleObj->GetTransform();

		//	// ★ ランダムスケール
		//	tf.SetScale(Vector3(
		//		static_cast<float>(rng.uniformReal(30.0f, 100.0f)),
		//		static_cast<float>(rng.uniformReal(30.0f, 100.0f)),
		//		static_cast<float>(rng.uniformReal(30.0f, 100.0f))
		//	));

		//	// ★ ランダム Y 回転
		//	float yrot = static_cast<float>(rng.uniformReal(-PI, PI));
		//	tf.SetRotation(Quaternion::CreateFromYawPitchRoll(yrot, 0.0f, 0.0f));

		//	// ★ ランダム位置（地形の高さに合わせる）
		//	Vector3 pos;
		//	pos.x = static_cast<float>(rng.uniformReal(-1750.0f, 1750.0f));
		//	pos.z = static_cast<float>(rng.uniformReal(-1750.0f, 1750.0f));
		//	pos.y = m_field->GetHeight2(pos);

		//	tf.SetPosition(pos);

		//	// 反映
		//	obstacleObj->SetTransform(tf);

		//	// Init() 実行（FallingBox と同じ順）
		//	obstacleObj->Init();

		//	// 配列に保持したいなら
		//	m_obstacles[cnt] = obstacleObj;
		//}

		// 敵
		// Instantiate
		auto enemyObj = m_pObjectManager->Instantiate<Enemy>("Enemy_" + std::to_string(0), Tag::Enemy);
		// 初期化
		// 配列に保持
		m_enemies[0] = enemyObj;
		//for(int cnt = 0; cnt < ENEMYMAX; cnt++)
		//{
		//	// Instantiate
		//	auto enemyObj = m_pObjectManager->Instantiate<Enemy>("Enemy" + std::to_string(cnt), Tag::Enemy);
		//	// 初期化
		//	enemyObj->Init();
		//	// 配列に保持
		//	m_enemies[cnt] = enemyObj;
		//}

		// 落下する箱
		//{
		//    auto fallingBox = m_pObjectManager->Instantiate<obstacle>("FallingBox", Tag::Object, this);

		//    Transform tf = fallingBox->GetTransform();
		//    tf.SetScale(Vector3(20.0f, 20.0f, 20.0f));    // 小さめ
		//    tf.SetPosition(Vector3(0.0f, 200.0f, 0.0f));  // 高所に配置
		//    tf.SetRotation(Quaternion::Identity);

		//    fallingBox->SetTransform(tf);
		//    fallingBox->Init();
		//}
	}

	// デバッグ Directional light
	DebugUI::RedistDebugFunction([this]() {
		debugDirectionalLight();
		});

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
