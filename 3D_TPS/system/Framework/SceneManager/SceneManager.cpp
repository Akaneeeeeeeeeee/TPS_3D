#include "SceneManager.h"
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "scene/SkeltalmeshScene.h"
#include "scene/TitleScene.h"
#include "scene/ResultScene.h"
#include "scene/TestScene.h"
#include "system/Framework/SceneManager/Transition/FadeTransition.h"

/**
 * @brief シーン配列初期化
*/
void SceneManager::Init(ObjectManager* _pObjectMgr)
{
	// オブジェクト管理クラスのポインタをセット
	m_pObjectManager = _pObjectMgr;

	this->AddScene<TitleScene>("TitleScene");
	this->AddScene<SkeltalmeshScene>("SkeltalmeshScene");
	this->AddScene<ResultScene>("ResultScene");
	this->AddScene<TestScene>("TestScene");
	//! タイトルシーンを生成してコンテナに追加
	//this->m_CurrentSceneName = "SkeltalmeshScene";
	this->m_CurrentSceneName = "TestScene";
	//this->m_CurrentSceneName = "TitleScene";
	// 現在のシーンを初期化
	m_pScenes[m_CurrentSceneName]->Init(m_pObjectManager);
	this->SetCurrentScene(m_CurrentSceneName);
}

/**
 * @brief 更新
*/
void SceneManager::Update(const float deltatime)
{
	// 現在シーンの更新
	//m_pScenes[m_CurrentSceneName]->Update(deltatime);

	//// 現在シーンの遷移フラグが立っている場合、シーン遷移
	//if (m_pScenes[m_CurrentSceneName]->GetChangeScene())
	//{
	//	// シーン遷移処理実行
	//	this->SetCurrentScene(m_pScenes[m_CurrentSceneName]->GetNextSceneName());
	//}

	// シーン遷移中でなければ通常更新
	if (IsSceneChanging && m_Transition)
	{
		// 遷移演出の更新
		m_Transition->update(deltatime);

		// 遷移演出が終了したら
		if (m_Transition->isFinished())
		{
			// 切り替え処理は終わってるのでフラグと演出オブジェクトをリセット
			m_Transition.release();
			IsSceneChanging = false;
		}
	}
	else if (m_pScenes[m_CurrentSceneName]->GetChangeScene())
	{
		// シーン遷移処理実行
		this->SetCurrentScene(m_pScenes[m_CurrentSceneName]->GetNextSceneName(), 
			std::make_unique<FadeTransition>(this, 2500.0f, FadeTransition::Mode::FadeInOut));
	}
	// 通常更新
	else if (!m_CurrentSceneName.empty())
	{
		m_pScenes[m_CurrentSceneName]->Update(deltatime);
	}
	
}

void SceneManager::Draw(void)
{
	//! 現在シーンによってそのシーンを描画
	//m_pScenes[m_CurrentSceneName]->Draw(deltatime);

	// 現在シーンが設定されていれば描画
	if (!m_CurrentSceneName.empty())
	{
		m_pScenes[m_CurrentSceneName]->Draw();
	}

	// シーン遷移中であれば遷移演出を描画
	if (IsSceneChanging && m_Transition)
	{
		m_Transition->draw();
	}
}

void SceneManager::Uninit(void)
{
	// 全てのシーンの終了処理
	for (auto& scene : m_pScenes)
	{
		// シーンの中身を解放
		scene.second.reset();
	}
	// シーン配列全体を解放
	this->m_pScenes.clear();
	// 名前も開放
	this->m_CurrentSceneName.clear();
}

/// <summary>
/// 指定されたシーン名に対応するシーンインスタンスを作成し、オブジェクト管理クラスを設定する
/// </summary>
/// <param name="_NewScene">作成するシーンの種類を示す SceneName 型の値</param>
void SceneManager::SetCurrentScene(const std::string& scenename, std::unique_ptr<SceneTransition> transition)
{
	//this->m_CurrentSceneName = scenename;
	//auto obj = SceneClassFactory::getInstance().create(scenename);
	//obj->Init(this->m_pObjectManager);
	//// ここで所有権がなくなるので自動で解放される
	//m_pScenes[m_CurrentSceneName] = std::move(obj);

	// 遷移演出が指定されている場合は演出から開始
	if (transition) {
		m_Transition = std::move(transition);
		m_Transition->start(scenename);
		IsSceneChanging = true;
		//m_pScenes[m_CurrentSceneName]->SetNextSceneName(scenename);
	}
	else {
		this->ChangeScene(scenename);
	}
}

/// <summary>
/// 指定されたシーン名に基づいて現在のシーンを変更を行う
/// </summary>
/// <param name="scenename">切り替え先のシーン名</param>
void SceneManager::ChangeScene(const std::string& nextscenename)
{
	// 指定されたシーン名が存在する(初期化時にAddSceneされている)場合にのみ切り替え
	if (m_pScenes.count(nextscenename))
	{
		// 現在のシーンがゲームシーンである場合、結果が必要なためキャストする
		bool isClear = false;
		if (m_CurrentSceneName == "SkeltalmeshScene")
		{
			SkeltalmeshScene* gameScene = static_cast<SkeltalmeshScene*>(m_pScenes[m_CurrentSceneName].get());
			isClear = gameScene->GetIsClear();
		}

		// 現在のシーンを終了
		if (!m_CurrentSceneName.empty())
		{
			m_pScenes[m_CurrentSceneName]->Uninit();
			// 現在のシーン名を変更
			m_CurrentSceneName = nextscenename;
			//m_pScenes[m_CurrentSceneName]->Init(m_pObjectManager);
		}

		// ゲームをクリアしていた場合
		if (isClear)
		{
			// リザルトシーンの画像を変更
			ResultScene* scene = static_cast<ResultScene*>(m_pScenes["ResultScene"].get());
			scene->SetTexture(std::make_unique<CSprite>
				(SCREEN_WIDTH, SCREEN_HEIGHT, "assets/texture/Images/GameClear.jpg")
			);
		}
	}
}

void SceneManager::SetSceneFactory(SceneClassFactory* factory)
{
	//m_pSceneFactory = factory;
}

void SceneManager::SetObjectManager(ObjectManager* manager)
{
	m_pObjectManager = manager;
}

