#include "SceneManager.h"
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/Scene/IScene.h"
#include "system/Framework/SceneManager/Transition/FadeTransition.h"
#include "system/SceneClassFactory.h"

SceneManager::SceneManager()
	: m_pObjectManager(nullptr),
	  m_CurrentScene(nullptr),
	  m_IsSceneChanging(false),
	  m_Transition(nullptr),
	  m_IsQuit(false),
	  m_SceneChangedInTransition(false)
{
}

SceneManager::~SceneManager()
{
	Uninit();
}

/**
 * @brief シーン配列初期化
*/
void SceneManager::Init(ObjectManager* object_manager, const std::string& first_scene_name)
{
	m_pObjectManager = object_manager;
	m_IsQuit = false;

	// 最初の1シーンだけ作成
	ChangeSceneImmediate(first_scene_name);
}

/*
* @brief Factoryを使ってシーンを生成し、初期化まで行う
* @param scene_name シーン名
* @return 生成されたシーン
*/
std::unique_ptr<IScene> SceneManager::CreateScene(const std::string& scene_name)
{
	auto& factory = SceneClassFactory::GetInstance();

	auto scene = factory.Create(scene_name);
	if (!scene)
	{
		// Scene 未登録時の対処
		// Logger::Error("Scene not found: " + scene_name);
		return nullptr;
	}

	// 依存を渡して初期化
	scene->Init(m_pObjectManager);
	return scene;
}

/*
* @brief シーン即時切り替え
* @param next_scene_name 切り替え先シーン名
* @remarks 遷移演出なしで即時にシーンを切り替える
*/
void SceneManager::ChangeSceneImmediate(const std::string& next_scene_name)
{
	// 旧シーン終了
	if (m_CurrentScene)
	{
		m_CurrentScene->Uninit();
		m_CurrentScene.reset();
	}

	// 新シーン生成
	auto new_scene = CreateScene(next_scene_name);
	if (!new_scene)
	{
		// ロード失敗時の対処。とりあえず何もしない。
		return;
	}

	m_CurrentSceneName = next_scene_name;
	m_CurrentScene = std::move(new_scene);

	// 遷移中フラグはここで下げておく
	m_IsSceneChanging = false;
}

/*
* @brief	シーン切り替え要求
* @param	next_scene_name 切り替え先シーン名
* @remarks	遷移演出あり/なしに応じてシーン切り替えを行う
*/
void SceneManager::RequestChangeScene(const std::string& next_scene_name,
	std::unique_ptr<SceneTransition> transition)
{
	// すでに遷移中なら上書きしない
	if (m_IsSceneChanging) { return; }

	m_NextSceneName = next_scene_name;
	m_IsSceneChanging = true;
	m_SceneChangedInTransition = false;

	// 演出がないなら即切り替え
	if (!transition)
	{
		ChangeSceneImmediate(m_NextSceneName);
		return;
	}

	// 演出あり
	m_Transition = std::move(transition);
	m_Transition->Start(m_NextSceneName);
}


/**
 * @brief 更新
*/
void SceneManager::Update(float delta_time)
{
	if (m_IsQuit) { return; }

	// 遷移中
	if (m_IsSceneChanging && m_Transition)
	{
		m_Transition->Update(delta_time);

		// 「ここでシーン切り替えてほしい」という合図が来たら一度だけ切り替え
		if (!m_SceneChangedInTransition && m_Transition->NeedsSceneChange())
		{
			ChangeSceneImmediate(m_NextSceneName);
			m_Transition->OnSceneChanged();
			m_SceneChangedInTransition = true;
		}

		// トランジションが完全に終わったら終了
		if (m_Transition->IsFinished())
		{
			m_Transition.reset();
			m_IsSceneChanging = false;
			m_SceneChangedInTransition = false;
		}
		return;
	}

	 // 通常更新
    if (m_CurrentScene)
    {
        m_CurrentScene->Update(delta_time);

        if (m_CurrentScene->GetChangeScene())
        {
            const std::string next = m_CurrentScene->GetNextSceneName();

            auto fade = std::make_unique<FadeTransition>(
				2500.0f,  // 2.5 秒
                FadeTransition::Mode::FadeInOut
            );
            RequestChangeScene(next, std::move(fade));
        }
    }
}

void SceneManager::Draw()
{
	// シーン描画
	if (m_CurrentScene)
	{
		m_CurrentScene->Draw();
	}

	// 遷移演出描画（フェードなど）
	if (m_IsSceneChanging && m_Transition)
	{
		m_Transition->Draw();
	}
}

void SceneManager::Uninit()
{
	// 現在シーンをきちんと終了
	if (m_CurrentScene)
	{
		m_CurrentScene->Uninit();
		m_CurrentScene.reset();
	}

	m_CurrentSceneName.clear();
	m_NextSceneName.clear();
	m_Transition.reset();

	m_IsSceneChanging = false;
	m_IsQuit = false;
	m_pObjectManager = nullptr;
}

/// <summary>
/// 指定されたシーン名に対応するシーンインスタンスを作成し、オブジェクト管理クラスを設定する
/// </summary>
/// <param name="_NewScene">作成するシーンの種類を示す SceneName 型の値</param>
//void SceneManager::SetCurrentScene(const std::string& scenename, std::unique_ptr<SceneTransition> transition)
//{
//	//this->m_CurrentSceneName = scenename;
//	//auto obj = SceneClassFactory::getInstance().create(scenename);
//	//obj->Init(this->m_pObjectManager);
//	//// ここで所有権がなくなるので自動で解放される
//	//m_pScenes[m_CurrentSceneName] = std::move(obj);
//
//	// 遷移演出が指定されている場合は演出から開始
//	if (transition) {
//		m_Transition = std::move(transition);
//		m_Transition->start(scenename);
//		IsSceneChanging = true;
//		//m_pScenes[m_CurrentSceneName]->SetNextSceneName(scenename);
//	}
//	else {
//		this->ChangeScene(scenename);
//	}
//}
//
///// <summary>
///// 指定されたシーン名に基づいて現在のシーンを変更を行う
///// </summary>
///// <param name="scenename">切り替え先のシーン名</param>
//void SceneManager::ChangeScene(const std::string& nextscenename)
//{
//	// 指定されたシーン名が存在する(初期化時にAddSceneされている)場合にのみ切り替え
//	if (m_pScenes.count(nextscenename))
//	{
//		// 現在のシーンがゲームシーンである場合、結果が必要なためキャストする
//		bool isClear = false;
//		if (m_CurrentSceneName == "SkeltalmeshScene")
//		{
//			SkeltalmeshScene* gameScene = static_cast<SkeltalmeshScene*>(m_pScenes[m_CurrentSceneName].get());
//			isClear = gameScene->GetIsClear();
//		}
//
//		// 現在のシーンを終了
//		if (!m_CurrentSceneName.empty())
//		{
//			m_pScenes[m_CurrentSceneName]->Uninit();
//			// 現在のシーン名を変更
//			m_CurrentSceneName = nextscenename;
//			//m_pScenes[m_CurrentSceneName]->Init(m_pObjectManager);
//		}
//
//		// ゲームをクリアしていた場合
//		if (isClear)
//		{
//			// リザルトシーンの画像を変更
//			ResultScene* scene = static_cast<ResultScene*>(m_pScenes["ResultScene"].get());
//			scene->SetTexture(std::make_unique<CSprite>
//				(SCREEN_WIDTH, SCREEN_HEIGHT, "assets/texture/Images/GameClear.jpg")
//			);
//		}
//	}
//}
//
//void SceneManager::SetSceneFactory(SceneClassFactory* factory)
//{
//	//m_pSceneFactory = factory;
//}
//
//void SceneManager::SetObjectManager(ObjectManager* manager)
//{
//	m_pObjectManager = manager;
//}
//
