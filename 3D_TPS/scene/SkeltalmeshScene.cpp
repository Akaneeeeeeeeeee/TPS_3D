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
SkeltalmeshScene::SkeltalmeshScene()
{
}

/**
 * @brief クリアシーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void SkeltalmeshScene::Update(uint64_t deltatime)
{
	m_panimobject->Update(1.0f);
	//m_pObjectManager->Update(deltatime);

	auto& input = CDirectInput::GetInstance();

	// --- カメラ回転 ---
	static float azimuth = m_camera.GetAzimuth();
	static float elevation = m_camera.GetElevation();
	if (input.GetMouseRButtonCheck()) {
		// マウスの相対移動量を取得
		//LONG dx = input.GetMouseStateData().lX;
		//LONG dy = input.GetMouseStateData().lY;

		//float sensitivity = 0.005f; // マウス感度

		//// アジマス(水平方向)と仰角(垂直方向)を更新
		//float newAzimuth = m_camera.GetAzimuth() + dx * sensitivity;
		//float newElevation = m_camera.GetElevation() - dy * sensitivity;

		//// 上下の制限を設定
		//const float limit = (PI / 2.0f) - 0.01f;
		//if (newElevation > limit) newElevation = limit;
		//if (newElevation < -limit) newElevation = -limit;

		//m_camera.SetAzimuth(newAzimuth);
		//m_camera.SetElevation(newElevation);
		LONG dx = input.GetMouseStateData().lX;
		LONG dy = input.GetMouseStateData().lY;

		float sensitivity = 0.005f;

		azimuth += dx * sensitivity;
		elevation -= dy * sensitivity;

		const float limit = (PI / 2.0f) - 0.01f;
		if (elevation > limit) elevation = limit;
		if (elevation < -limit) elevation = -limit;
	}

	// --- 移動入力 ---
	Vector3 move(0, 0, 0);

	if (input.CheckKeyBuffer(DIK_W)) {
		// キャラの forward 方向へ
		move.z += 1.0f;
	}
	if (input.CheckKeyBuffer(DIK_S)) {
		move.z -= 1.0f;
	}
	if (input.CheckKeyBuffer(DIK_A)) {
		move.x -= 1.0f;
	}
	if (input.CheckKeyBuffer(DIK_D)) {
		move.x += 1.0f;
	}
	// move に入力があるなら、キャラの向きを更新
	if (move.LengthSquared() > 0.0f) {
		move.Normalize();

		// 進行方向の角度を求める (XZ 平面)
		float targetYaw = std::atan2(-move.x, -move.z);

		// キャラの向きを更新（スナップ式）
		rotate.y = targetYaw;

		// 回転行列を作って move をワールド方向へ変換
		Matrix4x4 rotY = Matrix4x4::CreateRotationY(rotate.y);
		Vector3 forward = Vector3::TransformNormal(Vector3(0, 0, -1), rotY);

		pos += forward * moveSpeed;
	}
	else {
		// 入力がない場合はアニメーションをアイドル状態に変更
		//aiAnimation* animation = m_panimdata->GetAnimation("Idle", 0);
		//m_pmesh->SetCurentAnimation(animation);

	}

	Matrix4x4 mtxscale = Matrix4x4::CreateScale(scale);

	Matrix4x4 mtxrotx = Matrix4x4::CreateRotationX(rotate.x);
	Matrix4x4 mtxroty = Matrix4x4::CreateRotationY(rotate.y);
	Matrix4x4 mtxrotz = Matrix4x4::CreateRotationZ(rotate.z);

	Matrix4x4 mtxtrans = Matrix4x4::CreateTranslation(pos);

	// 描画時に使用する行列にまとめる
	m_mtxWorld = mtxscale * mtxrotx * mtxroty * mtxrotz * mtxtrans;

	// キャラを追従するようにカメラ座標調整
	//m_camera.SetLookat(pos);
	//m_camera.SetAzimuth(rotate.y);
	//m_camera.SetPosition(Vector3(pos.x, pos.y + 200, pos.z - 750));

	//m_camera.SetLookat(pos);  // キャラ位置を注視
	//m_camera.CalcCameraPositionTranslate(Vector3(pos.x, pos.y + 200, pos.z - 750)); // キャラを中心に回転
	m_camera.SetAzimuth(azimuth);
	m_camera.SetElevation(elevation);
	m_camera.SetLookat(pos);
	m_camera.CalcCameraPositionTranslate(pos);
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void SkeltalmeshScene::Draw(uint64_t deltatime)
{
	m_camera.Draw();

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
	m_panimobject->Draw();
	m_pTerrain->Draw();
	m_pSkydome->Draw();
	//m_pObjectManager->Draw(deltatime);
	

	// 目標方向の姿勢を作る
	AimOrientation aimorien(dir);
	aimorien.VisualizeDirection(
		Vector3(0, 0, 0),20,1,Color(1,1,0,1),2,Color(1,0,0,1)
	);
}

/**
 * @brief シーンの初期化処理
 */
void SkeltalmeshScene::Init(ObjectManager* _pObjectMgr)
{
	// オブジェクトマネージャーのセット
	//this->m_pObjectManager = _pObjectMgr;

	// カメラ(3D)の初期化
	m_camera.Init();

	// ローカル軸表示用線分の初期化
	m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	// メッシュを生成
	m_pmesh = std::make_unique<CAnimationMesh>();
	m_pmesh->Load(g_loadmodel[0].filename, g_loadmodel[0].texdirectoryname);

	// アニメーションオブジェクトを生成
	m_panimobject = std::make_unique<CAnimationObject>();
	m_panimobject->Init();	

	// アニメーションメッシュをセット
	m_panimobject->SetAnimationMesh(m_pmesh.get());

	// アニメーションデータ読み込み
	m_panimdata = std::make_unique<CAnimationData>();
	m_panimdata->LoadAnimation("assets/model/akai/Akai_Run.fbx","Run");
	//m_panimdata->LoadAnimation("assets/model/akai/Akai_Idle.fbx", "Idle");

	// 現在のアニメーションをセット
	aiAnimation* animation = m_panimdata->GetAnimation("Run", 0);
	m_pmesh->SetCurentAnimation(animation);

	// シェーダーの初期化
	m_shader.Create("shader/vertexLightingOneSkinVS.hlsl", "shader/vertexLightingPS.hlsl");

	//m_pObjectManager->CreateObject<Character>("testcharacter");


	// メッシュを生成
	m_arrowmesh = std::make_unique<CStaticMesh>();
	m_arrowmesh->Load(g_loadmodel[0].filename, g_loadmodel[0].texdirectoryname);
	m_arrowmeshrenderer = std::make_unique<CStaticMeshRenderer>();
	m_arrowmeshrenderer->Init(*m_arrowmesh);
	m_pTerrain = std::make_unique<Terrain>();
	m_pTerrain->Init(1, 1, 5000, 5000);
	m_pTerrain->SetImage("assets/texture/Hole1.png");
	m_pSkydome = std::make_unique<Skydome>();
	m_pSkydome->Init();
	m_pSkydome->SetTexture("assets/texture/haikei.jpg");

	// シェーダーの初期化
	m_arrowshader.Create(
		"shader/unlitTextureVS.hlsl",			// 頂点シェーダー
		"shader/unlitTexturePS.hlsl");			// ピクセルシェーダー


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

	m_camera.SetLookat(pos);
}

/**
 * @brief シーンの終了処理
 */
void SkeltalmeshScene::Uninit()
{
}
