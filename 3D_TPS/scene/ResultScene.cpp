#include <string>
#include <array>

#include "system/Framework/Application/Entry/main.h"
#include "system/CDirectInput.h"
#include "system/Framework/SceneManager/SceneManager.h"
#include "system/DebugUI.h"
#include "system/utility.h"
#include "system/AimOrientation.h"
#include "system/Framework/Component/UI/UIImageComponent.h"
#include "system/Framework/Component/Renderer/SpriteRenderer/UISpriteRenderer.h"
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

	auto* img = uiObj->AddComponent<UIImageComponent>("Image", tex);

	// 画面中央など
	auto& r = img->Rect();
	r.anchor = { 0.5f, 0.5f };
	r.anchoredPosPx = { 0, 0 };
	r.sizePx = { SCREEN_WIDTH, SCREEN_HEIGHT };
	r.pivot = { 0.5f, 0.5f };
	r.layer = 100;
	r.order = 0;
	r.visible = true;

	// 描画依頼を投げるrendererを追加
	auto* uiR = uiObj->AddComponent<UISpriteRenderer>("UIRenderer");
	uiR->SetSource(img);
	uiR->SetPhase(RenderPhase::Overlay2D);
	uiR->SetLayer(r.layer);
	uiR->SetOrder(r.order);
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