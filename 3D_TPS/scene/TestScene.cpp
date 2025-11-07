#include <string>
#include <array>

#include "TestScene.h"
#include "system/debugui.h"
#include "system/AimOrientation.h"
#include "system/SphereDrawer.h"
#include "system/ConeDrawer.h"
#include "system/LineDrawer.h"
#include "GameObject/Skydome.h"

#include "system/TriangleDrawer.h"
#include "system/meshmanager.h"
#include "system/RandomEngine.h"

namespace {
}

// 現在位置のフィールドの高さ表示
void TestScene::debugFieldHeight() {

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

// 平行光源の方向セット
void TestScene::debugDirectionalLight()
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
void TestScene::debugFreeCamera()
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
void TestScene::debugFieldRemake() {

	ImGui::Begin("debug Field Remake");

	static int dividex = 50;
	static int dividez = 50;

	static float width = 2500.0f;
	static float depth = 2500.0f;

	ImGui::SliderFloat("width", &width, 10, 1000);
	ImGui::SliderFloat("depth", &depth, 10, 1000);
	ImGui::SliderInt("devide x", &dividex, 1, 50);
	ImGui::SliderInt("devide z", &dividez, 1, 50);

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
void TestScene::debugFieldUnduration() {

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
TestScene::TestScene()
{
}

/**
 * @brief シーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void TestScene::Update(uint64_t deltatime)
{

	// プレイヤー更新
	m_player->Update(deltatime);

	// 現在位置取得
	Vector3 pos = m_player->GetTransform().GetPosition();

	// 地形に合わせて高さ修正
	float height = m_field->GetHeight(pos);
	pos.y = height;

	// 修正した位置を反映
	m_player->SetPosition(pos);

	// 敵
	for (auto& enemy : m_enemies) {
		enemy->Update(deltatime);

		Vector3 pos = enemy->GetPosition();
		float height = m_field->GetHeight2(pos);
		pos.y = height;

		enemy->SetPosition(pos);
	}

	// 障害物
	for (auto& obs : m_obstacles) {

		Vector3 pos = obs->GetTransform().GetPosition();
		float height = m_field->GetHeight2(pos);
		pos.y = height;

		obs->SetPosition(pos);
	}
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void TestScene::Draw(void)
{
	m_camera.Draw();

	// 3軸カラー
	Color axiscol[3] = {
		Color(1, 0, 0, 1),
		Color(0, 1, 0, 1),
		Color(0, 1, 1, 1)
	};

	// ワールド軸を描画
	SetLineWidth(1.0f);					// 太さを設定
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}

	// フィールド
//	Renderer::SetFillMode(D3D11_FILL_WIREFRAME);
	//m_field->Draw();
	//	Renderer::SetFillMode(D3D11_FILL_SOLID);

	// プレイヤ
	//m_player->Draw();

	// 平行光源の方向を示す矢印を描画 
//	LIGHT l = Renderer::GetLight();
//	Vector3 dir = Vector3(-l.Direction.x, -l.Direction.y, -l.Direction.z);

//	AimOrientation aimorien(dir);
//	aimorien.VisualizeDirection(
//		Vector3(0, 100, 0), 20, 1, Color(1, 1, 0, 1), 2, Color(1, 0, 0, 1)
//	);

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

	// 敵
	//for (auto& enemy : m_enemies) {
	//	enemy->Draw();
	//}

	//// 障害物
	//for (auto& obs : m_obstacles) {
	//	obs->Draw();
	//}
}

/**
 * @brief シーンの初期化処理
 */
void TestScene::Init(ObjectManager* mgr)
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

	// メッシュデータ読み込み（敵用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/car001.x", "assets/model/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("car001.x", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("car001.x", std::move(srenderer));
	}

	// メッシュデータ読み込み（障害物用）
	{
		std::unique_ptr<CStaticMesh> smesh = std::make_unique<CStaticMesh>();
		smesh->Load("assets/model/obj/box.obj", "assets/model/obj/");

		std::unique_ptr<CStaticMeshRenderer> srenderer = std::make_unique<CStaticMeshRenderer>();
		srenderer->Init(*smesh);

		MeshManager::RegisterMesh<CStaticMesh>("obstaclebox", std::move(smesh));
		MeshManager::RegisterMeshRenderer<CStaticMeshRenderer>("obstaclebox", std::move(srenderer));
	}


	// フィールド初期化
	m_field = m_pObjectManager->Instantiate<Terrain>("field", Tag::Field);
	m_field->Init();

	// プレイヤ
	m_player = m_pObjectManager->Instantiate<Player>("player", Tag::Player);
	m_player->Init();
	m_player->SetCamera(&m_camera);

	// スカイドーム
	auto skydome = m_pObjectManager->Instantiate<Skydome>("skydome", Tag::Object);
	skydome->Init();
	skydome->SetTexture("assets/texture/haikei.jpg");

	// 敵群初期化
	{
		auto& rng = RandomEngine::tls();
		rng.uniformReal(-500, 500);

		for (int cnt = 0; cnt < ENEMYMAX; cnt++) {
			auto player = m_pObjectManager->GetObjectByName<Player>("player");

			m_enemies[cnt] = m_pObjectManager->Instantiate<Enemy>("Enemy" + std::to_string(cnt), Tag::Enemy, player);
			m_enemies[cnt]->Init();

			Transform tf = m_enemies[cnt]->GetTransform();

			tf.SetScale(Vector3(1, 1, 1));
			tf.SetRotation(Quaternion::Identity);

			Vector3 pos(
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f)),
				0,
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f))
			);

			float height = m_field->GetHeight2(pos);
			pos.y = height;
			tf.SetPosition(pos);

			m_enemies[cnt]->SetTransform(tf);
		}
	}

	// 障害物群初期化
	{
		auto& rng = RandomEngine::tls();
		rng.uniformReal(-500, 500);

		for (int cnt = 0; cnt < OBSTACLEMAX; cnt++) {

			m_obstacles[cnt] = m_pObjectManager->Instantiate<obstacle>("Obstacle" + std::to_string(cnt), Tag::Object, this);
			m_obstacles[cnt]->Init();

			Transform tf = m_obstacles[cnt]->GetTransform();

			tf.SetScale(Vector3(
				static_cast<float>(rng.uniformReal(10.0f, 30.0f)),
				static_cast<float>(rng.uniformReal(10.0f, 30.0f)),
				static_cast<float>(rng.uniformReal(10.0f, 30.0f))
			));

			float yrot = static_cast<float>(rng.uniformReal(-PI, PI));
			tf.SetRotation(Quaternion::CreateFromYawPitchRoll(yrot, 0, 0));

			Vector3 pos(
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f)),
				0,
				static_cast<float>(rng.uniformReal(-500.0f, 500.0f))
			);
			pos.y = m_field->GetHeight2(pos);

			tf.SetPosition(pos);

			m_obstacles[cnt]->SetTransform(tf);
		}
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
void TestScene::Uninit()
{
}
