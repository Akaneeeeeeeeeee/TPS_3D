#include "TestScene.h"
#include "GameObject/Player/testPlayer.h"

TestScene::TestScene() : IScene()
{
	m_NextSceneName = "SkeltalmeshScene";
}
//TestScene::TestScene(ObjectManager& _Mgr) : IScene(_Mgr)
//{
//	m_NextSceneName = "SkeltalmeshScene";
//}

/**
 * @brief クリアシーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void TestScene::Update(uint64_t deltatime)
{
	// キーボードの状態を取得
	/*if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
	{
		this->ChangeScene = true;
	}*/
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void TestScene::Draw(void)
{

	// 描画時に使用する行列にまとめる
	//m_mtxWorld = Matrix4x4::Identity;

	m_camera.Draw();

	//Renderer::SetWorldMatrix(&m_mtxWorld);

	// タイトル画像の描画
	Vector3 pos = Vector3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1.0f);
	Vector3 rot = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
	//m_TitleImage->Draw(m_mtxWorld);
}

/**
 * @brief シーンの初期化処理
 */
void TestScene::Init(ObjectManager* _Mgr)
{
	// オブジェクトマネージャーのセット
	this->m_pObjectManager = _Mgr;

	// カメラ(3D)の初期化
	m_camera.Init();

	// ローカル軸表示用線分の初期化
	//m_segments[0] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(100, 0, 0));
	//m_segments[1] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 100, 0));
	//m_segments[2] = std::make_unique<Segment>(Vector3(0, 0, 0), Vector3(0, 0, 100));

	// プレイヤー生成
	auto player = m_pObjectManager->Instantiate<TestPlayer>("testPlayer", Tag::Player);
	player->Init();
	//player->SetCamera(&m_camera);

	

}

/**
 * @brief シーンの終了処理
 */
void TestScene::Uninit()
{
	this->ChangeScene = false;
}
