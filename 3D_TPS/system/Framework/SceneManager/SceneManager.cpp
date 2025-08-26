#include "SceneManager.h"

/**
 * @brief シーン配列初期化
*/
void SceneManager::Init(void) 
{
	//! タイトルシーンを生成してコンテナに追加
	this->CreateSceneInstance(SceneName::TITLE);
	//! Scenes現在シーンをタイトルシーンに設定
	m_CurrentSceneName = SceneName::TITLE;
	//! シーンの初期化
	m_pScenes[m_CurrentSceneName]->Init();
}

/**
 * @brief 更新
*/
void SceneManager::Update(void)
{
	// 現在シーンの更新
	m_pScenes[m_CurrentSceneName]->Update();

	// 現在シーンの遷移フラグが立っている場合、シーン遷移
	if (m_pScenes[m_CurrentSceneName]->GetChangeScene())
	{
		// シーン遷移処理実行
		ChangeScene(m_pScenes[m_CurrentSceneName]->GetRequestSceneName());
	}
}

void SceneManager::Draw(void)
{
	//! 現在シーンによってそのシーンを描画
	m_pScenes[m_CurrentSceneName]->Draw();
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
	m_pScenes.clear();
}

/// <summary>
/// 指定されたシーン名に対応するシーンインスタンスを作成し、オブジェクト管理クラスを設定する
/// </summary>
/// <param name="_NewScene">作成するシーンの種類を示す SceneName 型の値</param>
void SceneManager::CreateSceneInstance(SceneName _NewScene)
{
	switch (_NewScene)
	{
	case SceneName::TITLE:
		m_pScenes[_NewScene] = std::make_unique<TitleScene>();
		break;
		/*case SceneName::GAME:
			m_pScenes[_NewScene] = std::make_unique<GameScene>();
			break;*/
	case SceneName::RESULT:
		m_pScenes[_NewScene] = std::make_unique<ResultScene>();
		break;
	default:
		break;
	}
	// オブジェクト管理クラスを設定
	m_pScenes[_NewScene]->SetObjectManager(m_pObjectManager);
}

/**
 * @brief シーン切り替え関数
 * タイトル、終了画面（？）、ステージ選択シーンはシーンとして保持し続けておきたい（頻繁に使うため、毎回生成→解放しなくても良くない？）
 * @param  次のシーンタグ
*/
void SceneManager::ChangeScene(SceneName _NextScene)
{
	// 切り替え前のシーンステージ選択画面ではない（保持しておきたいシーンではない）場合、
	//if (m_CurrentSceneName != SceneName::SCENE_MAX)
	//{
	//	// 現在シーンを解放
	//	DeleteScene(m_CurrentSceneName);
	//}

	if (_NextScene == SceneName::NONE) {
		this->IsQuit = true;
		return;
	}

	// 現シーン削除（残しておきたいなら条件でスキップ）
	m_pScenes.erase(m_CurrentSceneName);

	// 次のシーンが未生成なら生成
	if (m_pScenes.find(_NextScene) == m_pScenes.end()) {
		CreateSceneInstance(_NextScene);
	}

	// 次のシーンを現在のシーンに設定し、初期化
	m_CurrentSceneName = _NextScene;
	m_pScenes[m_CurrentSceneName]->Init();
}

/**
 * @brief シーン削除関数
 * @param _SceneName 削除したいシーンの型（mapのキー）
 * 3. Init の呼び直しタイミング
切り替え時に Init を呼んでますが、再プレイ時などで状態をリセットしないシーンもあるかもしれません。
Init を呼ぶかどうかもシーンごとに制御できると便利です。

4. NONE で終了するのはシーン側でフラグを立てさせる
NONE というシーン名で終了扱いはちょっと不自然なので、
IScene に bool IsQuitRequested() みたいな関数を用意して SceneManager がそれを見る形にした方が直感的です。
*/
void SceneManager::DeleteScene(SceneName _SceneName)
{
	// 指定したシーンを削除
	m_pScenes.erase(_SceneName);
}

