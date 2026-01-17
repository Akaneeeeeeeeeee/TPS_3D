#include <string>
#include <array>

#include "system/Framework/Application/Entry/main.h"
#include "system/CDirectInput.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/DebugUI.h"
#include "system/utility.h"
#include "system/AimOrientation.h"
#include "system/Framework/Component/UI/UIImageComponent.h"
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
}

/**
 * @brief シーンの初期化処理
 */
void ResultScene::Init(ObjectManager* _Mgr)
{

	m_pObjectManager = _Mgr;

	ResultType t = ResultType::None;
	if (m_pObjectManager) t = m_pObjectManager->GetGameResult();

	const char* tex = "assets/texture/default.png"; // デフォルト
	if (t == ResultType::Clear)  tex = "assets/texture/Images/GameClear.jpg";
	if (t == ResultType::Found)  tex = "assets/texture/Images/GameOver.jpg";
	if (t == ResultType::TimeUp) tex = "assets/texture/Images/TimeUp.jpg";

	// UI専用オブジェクト
	auto* uiObj = m_pObjectManager->Instantiate<GameObject>(
		"UI_TitleLogo",
		Tag::Object,              // Tag::UI があるならそれでもOK
		Transform::One()
	);

	UIImageComponent* img = uiObj->AddComponent<UIImageComponent>(
		"Image",
		tex
	);

	// UITransform（内包Rect）をセット
	auto& r = img->Rect();

	// 画面中央に置く
	r.anchor = { 1.0f, 0.f };          // 基準点：画面中央
	r.anchoredPosPx = { 0.0f, 0.0f };   // 基準点からの差分(px)
	r.sizePx = { SCREEN_WIDTH, SCREEN_HEIGHT };      // 表示サイズ(px)
	r.pivot = { 0.5f, 0.5f };          // 自分の中心を基準
	r.rotZRad = 0.0f;

	r.layer = 100;
	r.order = 0;
	r.visible = true;
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