#include <string>
#include <array>

#include "system/Framework/Application/Entry/main.h"
#include "system/CDirectInput.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/DebugUI.h"
#include "system/utility.h"
#include "system/AimOrientation.h"

#include "SkeltalmeshScene.h"
#include "system/Framework/GameObject/Character/Character.h"

//struct Load3DInfo{
//	std::string filename;
//	std::string texdirectoryname;
//	Load3DInfo(std::string p1, std::string p2) {
//		filename = p1;
//		texdirectoryname = p2;
//	}
//};

std::array<Load3DInfo,1> g_loadmodel = 
{
		Load3DInfo(
			"assets/model/akai/akai.fbx",			// モデル名
			"assets/model/akai/")					// テクスチャのパス
};

// 平行光源の方向セット
void SkeltalmeshScene::debugDirectionalLight()
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
void SkeltalmeshScene::debugFreeCamera()
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

// デバッグModel select
void SkeltalmeshScene::debug3DModelSelect()
{
	ImGui::Begin("debug Shape Select");

	// 選択中のインデックス
	static int current_item = 0;
	static int old_item = -1;

	// アイテムのリスト
	const char* items[] = {
		g_loadmodel[0].filename.c_str(),
//		g_loadmodel[1].filename.c_str(),
//		g_loadmodel[2].filename.c_str(),
//		g_loadmodel[3].filename.c_str(),
//		g_loadmodel[4].filename.c_str(),
//		g_loadmodel[5].filename.c_str(),
//		g_loadmodel[6].filename.c_str(),
//		g_loadmodel[7].filename.c_str(),
//		g_loadmodel[8].filename.c_str(),
	};

	ImGui::Text("\n\n");
	ImGui::Separator();

	ImGui::Text("%s",g_loadmodel[current_item].filename.c_str());
	ImGui::Text("\n\n");

	ImGui::Separator();
	// 頂点数　三角形数　サブセット数　マテリアル数
	ImGui::Text("vertex num : %d", m_pmesh->GetVertices().size());
	ImGui::Text("triangle num : %d", m_pmesh->GetIndices().size()/3);
	ImGui::Text("subset num : %d", m_pmesh->GetSubsets().size());
	ImGui::Text("material num : %d", m_pmesh->GetMaterials().size());

	ImGui::End();
}

// デバッグSRT
void SkeltalmeshScene::debugSRT()
{
	ImGui::Begin("debug SRT");

	static Vector3 scale = Vector3(1, 1, 1);
	static Vector3 rotate = Vector3(0, 0, 0);
	static Vector3 trans = Vector3(0, 0, 0);

	ImGui::SliderFloat3("scale", &scale.x, 0.1f, 20.0f);
	ImGui::SliderFloat3("rotate", &rotate.x, -PI, PI);
	ImGui::SliderFloat3("trans", &trans.x, -100, 100);

	Matrix4x4 mtxscale = Matrix4x4::CreateScale(scale);

	Matrix4x4 mtxrotx = Matrix4x4::CreateRotationX(rotate.x);
	Matrix4x4 mtxroty = Matrix4x4::CreateRotationY(rotate.y);
	Matrix4x4 mtxrotz = Matrix4x4::CreateRotationZ(rotate.z);

	Matrix4x4 mtxtrans = Matrix4x4::CreateTranslation(trans);

	// 描画時に使用する行列にまとめる
	m_mtxWorld = mtxscale * mtxrotx * mtxroty * mtxrotz * mtxtrans;

	static int selected = 0;		// 0;SOLID 1:WIREFRAME

	ImGui::RadioButton("Solid", &selected, 0);
	ImGui::RadioButton("WireFrame", &selected, 1);

	if (selected == 0) {
		Renderer::SetFillMode(D3D11_FILL_SOLID);
	}
	else {
		Renderer::SetFillMode(D3D11_FILL_WIREFRAME);
	}

	ImGui::End();
}

/**
 * @brief コンストラクタ
 */
SkeltalmeshScene::SkeltalmeshScene(ObjectManager& _Mgr) : IScene(_Mgr)
{
	m_NextSceneName = "ResultScene";
}

/**
 * @brief クリアシーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void SkeltalmeshScene::Update(uint64_t deltatime)
{
	//m_pObjectManager->Update(deltatime);

	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
	{
		this->ChangeScene = true;
	}

	Matrix4x4 mtxscale = Matrix4x4::CreateScale(scale);

	Matrix4x4 mtxrotx = Matrix4x4::CreateRotationX(rotate.x);
	Matrix4x4 mtxroty = Matrix4x4::CreateRotationY(rotate.y);
	Matrix4x4 mtxrotz = Matrix4x4::CreateRotationZ(rotate.z);

	Matrix4x4 mtxtrans = Matrix4x4::CreateTranslation(pos);

	// 描画時に使用する行列にまとめる
	m_mtxWorld = mtxscale * mtxrotx * mtxroty * mtxrotz * mtxtrans;

	m_pCharacter->Update(deltatime);
	m_pBillboard->Update(deltatime);

	// 敵の更新
	for(size_t i = 0; i < m_pEnemies.size(); i++)
	{
		m_pEnemies[i]->Update(deltatime);
	}

	// 敵がプレイヤーを発見したかのチェック
	for(size_t i = 0; i < m_pEnemies.size(); i++)
	{
		// 見つかったらクリア状態に変更し、シーン遷移フラグを立てる
		if(m_pEnemies[i]->CanSeePlayer(m_pCharacter->GetPosition()))
		{
			this->ChangeScene = true;
			this->IsClear = false;
		}
	}

	// ビルボードとキャラが触れればクリア
	auto charaPos = m_pCharacter->GetPosition();
	auto billPos = m_pBillboard->GetPosition();
	// ビルボードの中心から一定範囲内にいればクリア
	if((charaPos - billPos).Length() < 300.0f)
	{
		this->ChangeScene = true;
		this->IsClear = true;
	}
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void SkeltalmeshScene::Draw(uint64_t deltatime)
{
	//m_camera.Draw();

	// 3軸カラー
	Color axiscol[3] = {
		Color(1, 0, 0, 1),
		Color(0, 1, 0, 1),
		Color(0, 1, 1, 1)
	};

	// ワールド軸を描画
	for (int axisno = 0; axisno < 3; axisno++)
	{
		Matrix4x4 rotmtx = Matrix4x4::Identity;
		m_segments[axisno]->Draw(rotmtx, axiscol[axisno]);
	}

	// ローカル軸を描画
	for (int axisno = 0; axisno < 3; axisno++)
	{
		m_segments[axisno]->Draw(m_mtxWorld, axiscol[axisno]);
	}

	// 平行光源の方向を示す矢印を描画 
	LIGHT l = Renderer::GetLight();
	Vector3 dir = Vector3(-l.Direction.x, -l.Direction.y, -l.Direction.z);

	// 図形の描画（ANIMMESH描画）
	Renderer::SetWorldMatrix(&m_mtxWorld);

	//LIGHT light{};
	//light.Enable = true;
	//light.Direction = Vector4(0.5f, -1.0f, 0.8f, 0.0f);
	//light.Direction.Normalize();
	//light.Ambient = Color(0.2f, 0.2f, 0.2f, 1.0f);
	//light.Diffuse = Color(1.5f, 1.5f, 1.5f, 1.0f);
	//ShaderManager::GetInstance().GetShader("vertexLightingOneSkinVS")->WriteCBuffer(4, &light);
	//Matrix4x4 view = m_camera.GetViewMatrix();
	//Matrix4x4 proj = m_camera.GetProjMatrix();
	//auto transposedWorld = m_mtxWorld.Transpose();
	//ShaderManager::GetInstance().GetShader("vertexLightingOneSkinVS")->WriteCBuffer(0, &m_mtxWorld);
	//auto transposedView = view.Transpose();
	//ShaderManager::GetInstance().GetShader("vertexLightingOneSkinVS")->WriteCBuffer(1, &transposedView);
	//auto transposedProj = proj.Transpose();
	//ShaderManager::GetInstance().GetShader("vertexLightingOneSkinVS")->WriteCBuffer(2, &transposedProj);

	m_shader.SetGPU();
	m_pBillboard->Draw(deltatime);

	m_pTerrain->Draw(deltatime);
	m_pSkydome->Draw(deltatime);
	//m_pObjectManager->Draw(deltatime);
	m_pCharacter->Draw(deltatime);

	for(size_t i = 0; i < m_pEnemies.size(); i++)
	{
		m_pEnemies[i]->Draw(deltatime);
	}

	// 目標方向の姿勢を作る
	AimOrientation aimorien(dir);
	aimorien.VisualizeDirection(
		Vector3(0, 0, 0),20,1,Color(1,1,0,1),2,Color(1,0,0,1)
	);
}

/**
 * @brief シーンの初期化処理
 */
void SkeltalmeshScene::Init(void)
{
	// オブジェクトマネージャーのセット
	//this->m_pObjectManager = _pObjectMgr;

	// カメラ(3D)の初期化
	m_camera.Init();

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	// プレイヤー生成
	m_pCharacter = std::make_unique<Player>();
	m_pCharacter->Init();
	m_pCharacter->SetCamera(&m_camera);

	// 敵生成
	for (size_t i = 0; i < m_pEnemies.size(); i++)
	{
		m_pEnemies[i] = std::make_unique<Enemy>();
		m_pEnemies[i]->Init();
	}

	//m_pObjectManager->CreateObject<Character>("testcharacter");

	// メッシュを生成
	m_pTerrain = std::make_unique<Terrain>();
	m_pTerrain->Init(50, 50, 5000, 5000);
	m_pTerrain->SetImage("assets/texture/Hole1.png");
	m_pSkydome = std::make_unique<Skydome>();
	m_pSkydome->Init();
	m_pSkydome->SetTexture("assets/texture/haikei.jpg");

	// ビルボード生成
	m_pBillboard = std::make_unique<Billboard>(300, 300, "assets/texture/emblem.png", &m_camera);
	m_pBillboard->Init();
	m_pBillboard->SetPosition(Vector3(0, 150, 3000));

	// 地形生成
	//m_pplanemesh = std::make_unique<CPlaneMesh>();
	//m_pplanemesh->Init(
	//	20, 20,					// 分割数
	//	200, 200,				// 幅、高さ
	//	Color(0.5f, 0.5f, 0.5f, 1.0f),	// 色
	//	Vector3(0, 1, 0),		// 法線
	//	true,					// XZ平面フラグ
	//	true);					// 時計回りフラグ

	// デバッグSRT
	DebugUI::RedistDebugFunction([this]() {
		debugSRT();
		});


	// デバッグ Directional light
	DebugUI::RedistDebugFunction([this]() {
		debugDirectionalLight();
		});

	// デバッグ Free Camera
	DebugUI::RedistDebugFunction([this]() {
		debugFreeCamera();
		});
}

/**
 * @brief シーンの終了処理
 */
void SkeltalmeshScene::Uninit()
{
	this->ChangeScene = false;
}
