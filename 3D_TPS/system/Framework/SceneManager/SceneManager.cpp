#include "SceneManager.h"
#include "system/Framework/ObjectManager/ObjectManager.h"

/**
 * @brief シーン配列初期化
*/
void SceneManager::Init(ObjectManager* _pObjectMgr)
{
	// オブジェクト管理クラスのポインタをセット
	m_pObjectManager = _pObjectMgr;
	//! タイトルシーンを生成してコンテナに追加
	this->m_CurrentSceneName = "SkeltalmeshScene";
	//this->m_CurrentSceneName = "TitleScene";
	this->SetCurrentScene(m_CurrentSceneName);
}

/**
 * @brief 更新
*/
void SceneManager::Update(uint64_t deltatime)
{
	// 現在シーンの更新
	m_pScenes[m_CurrentSceneName]->Update(deltatime);

	// 現在シーンの遷移フラグが立っている場合、シーン遷移
	if (m_pScenes[m_CurrentSceneName]->GetChangeScene())
	{
		// シーン遷移処理実行
		this->SetCurrentScene(m_pScenes[m_CurrentSceneName]->GetNextSceneName());
	}
}

void SceneManager::Draw(uint64_t deltatime)
{
	//! 現在シーンによってそのシーンを描画
	m_pScenes[m_CurrentSceneName]->Draw(deltatime);
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
void SceneManager::SetCurrentScene(const std::string& scenename)
{
	this->m_CurrentSceneName = scenename;
	auto obj = SceneClassFactory::getInstance().create(scenename);
	obj->Init(this->m_pObjectManager);
	// ここで所有権がなくなるので自動で解放される
	m_pScenes[m_CurrentSceneName] = std::move(obj);
}

void SceneManager::SetSceneFactory(SceneClassFactory* factory)
{
	//m_pSceneFactory = factory;
}

void SceneManager::SetObjectManager(ObjectManager* manager)
{
	m_pObjectManager = manager;
}

