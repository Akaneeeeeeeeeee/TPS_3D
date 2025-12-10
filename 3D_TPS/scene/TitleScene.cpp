#include <string>
#include <array>

#include "system/Framework/Application/Entry/main.h"
#include "system/CDirectInput.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/DebugUI.h"
#include "system/utility.h"
#include "system/AimOrientation.h"
#include "TitleScene.h"

// 平行光源の方向セット
void TitleScene::debugDirectionalLight()
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
void TitleScene::debugFreeCamera()
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
void TitleScene::debug3DModelSelect()
{
	ImGui::Begin("debug Shape Select");

	// 選択中のインデックス
	static int current_item = 0;
	static int old_item = -1;

	// アイテムのリスト
	//const char* items[] = {
	//	g_loadmodel2[0].filename.c_str(),
	//			g_loadmodel[1].filename.c_str(),
	//			g_loadmodel[2].filename.c_str(),
	//			g_loadmodel[3].filename.c_str(),
	//			g_loadmodel[4].filename.c_str(),
	//			g_loadmodel[5].filename.c_str(),
	//			g_loadmodel[6].filename.c_str(),
	//			g_loadmodel[7].filename.c_str(),
	//			g_loadmodel[8].filename.c_str(),
	//};

	ImGui::Text("\n\n");
	ImGui::Separator();

	//ImGui::Text("%s", g_loadmodel[current_item].filename.c_str());
	//ImGui::Text("\n\n");

	//ImGui::Separator();
	//// 頂点数　三角形数　サブセット数　マテリアル数
	//ImGui::Text("vertex num : %d", m_pmesh->GetVertices().size());
	//ImGui::Text("triangle num : %d", m_pmesh->GetIndices().size() / 3);
	//ImGui::Text("subset num : %d", m_pmesh->GetSubsets().size());
	//ImGui::Text("material num : %d", m_pmesh->GetMaterials().size());

	ImGui::End();
}

// デバッグSRT
void TitleScene::debugSRT()
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
TitleScene::TitleScene() : IScene()
{
	m_NextSceneName = "SkeltalmeshScene";
}
//TitleScene::TitleScene(ObjectManager& _Mgr) : IScene(_Mgr)
//{
//	m_NextSceneName = "SkeltalmeshScene";
//}

/**
 * @brief クリアシーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void TitleScene::Update(const float deltatime)
{
	// キーボードの状態を取得
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
	{
		this->ChangeScene = true;
	}
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void TitleScene::Draw(void)
{

	// 描画時に使用する行列にまとめる
	m_mtxWorld = Matrix4x4::Identity;

	Renderer::SetWorldMatrix(&m_mtxWorld);

	// タイトル画像の描画
	Vector3 pos = Vector3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1.0f);
	Vector3 rot = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
	m_TitleImage->Draw(scale, rot, pos);
	//m_TitleImage->Draw(m_mtxWorld);
}

/**
 * @brief シーンの初期化処理
 */
void TitleScene::Init(ObjectManager* _Mgr)
{
	// オブジェクトマネージャのセット
	this->m_pObjectManager = _Mgr;

	// カメラ(3D)の初期化
	m_camera.Init();

	// タイトル画像の生成
	m_TitleImage = std::make_unique<CSprite>(SCREEN_WIDTH, SCREEN_HEIGHT, "assets/texture/Images/HideAndSeek.jpg");
}

/**
 * @brief シーンの終了処理
 */
void TitleScene::Uninit()
{
	this->ChangeScene = false;
}
