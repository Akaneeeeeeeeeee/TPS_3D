#include <string>
#include <array>

#include "system/Framework/Application/Entry/main.h"
#include "system/CDirectInput.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/DebugUI.h"
#include "system/utility.h"
#include "system/AimOrientation.h"

#include "ResultScene.h"



/**
 * @brief コンストラクタ
 */
ResultScene::ResultScene() : IScene()
{
	m_NextSceneName = "AnimatedTitleScene";
}

/**
 * @brief クリアシーンの更新処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void ResultScene::Update(const float deltatime)
{
	// キーボードの状態を取得
	if (CDirectInput::GetInstance().CheckKeyBuffer(DIK_RETURN))
	{
		this->ChangeScene();
	}
}

/**
 * @brief 描画処理
 *
 * @param deltatime 前フレームからの経過時間（ミリ秒）
 */
void ResultScene::Draw(void)
{

	// 描画時に使用する行列にまとめる
	m_mtxWorld = Matrix4x4::Identity;

	Renderer::SetWorldMatrix(&m_mtxWorld);

	// タイトル画像の描画
	Vector3 pos = Vector3(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1.0f);
	Vector3 rot = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
	m_ResultImage->Draw(scale, rot, pos);
	//m_ResultImage->Draw(m_mtxWorld);
}

/**
 * @brief シーンの初期化処理
 */
void ResultScene::Init(ObjectManager* _Mgr)
{

	// リザルト画像の生成
	m_ResultImage = std::make_unique<CSprite>(SCREEN_WIDTH, SCREEN_HEIGHT, "assets/texture/Images/GameOver.jpg");
}

/**
 * @brief シーンの終了処理
 */
void ResultScene::Uninit()
{
	this->SetChangeScene(false);
}

void ResultScene::SetTexture(std::unique_ptr<CSprite> _sprite)
{
	// 背景画像を変更
	m_ResultImage = std::move(_sprite);
}