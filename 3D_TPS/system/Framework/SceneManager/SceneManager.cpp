#include "SceneManager.h"
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/Scene/IScene.h"
#include "system/Framework/SceneManager/Transition/FadeTransition.h"
#include "system/SceneClassFactory.h"
#include "system/DebugUI.h"
#include "Framework/Time/Time.h"

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

#ifdef _DEBUG
	DebugUI::RedistDebugFunction([this]() { this->DebugImGui(); });
#endif
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

// 切り替え本体（旧シーン Uninit → 新シーン Init）
// ・この関数は「フラグをいじらない」ことがポイント
void SceneManager::SwitchSceneCore(const std::string& next_scene_name)
{
	// 旧シーン終了
	if (m_CurrentScene)
	{
		m_CurrentScene->Uninit();
		// 旧シーンのオブジェクトをここで削除
		if (m_pObjectManager)
		{
			m_pObjectManager->DestroySceneObjects(m_CurrentSceneName);
			m_pObjectManager->FlushDestroyQueue();
		}
		m_CurrentScene.reset();
	}

	// 新しいシーン名を ObjectManager に教えておく
	if (m_pObjectManager)
	{
		m_pObjectManager->SetCurrentSceneName(next_scene_name);
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
}

/*
* @brief シーン即時切り替え
* @param next_scene_name 切り替え先シーン名
* @remarks 遷移演出なしで即時にシーンを切り替える
*/
// 「演出なし」の即時切り替え専用
void SceneManager::ChangeSceneImmediate(const std::string& next_scene_name)
{
	m_PendingCommit = false;
	SwitchSceneCore(next_scene_name);

	if(m_pObjectManager)
	{
		m_pObjectManager->FlushAwakeQueue();
		m_pObjectManager->FlushStartQueue();
	}

	// 即時切り替えなので遷移状態はリセット
	m_IsSceneChanging = false;
	m_SceneChangedInTransition = false;
	m_Transition.reset();
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
		m_PendingCommit = true;
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

	// 1. 遷移演出中
	if (m_IsSceneChanging && m_Transition)
	{
		m_Transition->Update(delta_time);

		// フェードアウト完了など「今切り替えてほしい」タイミング
		if (!m_SceneChangedInTransition && m_Transition->NeedsSceneChange())
		{
			// まだ切り替えず、Draw 後に切り替えるフラグを立てる
			m_PendingCommit = true;
		}

		// フェードインまで含めて演出が完全に終わったらリセット
		if (m_Transition->IsFinished())
		{
			m_Transition.reset();
			m_IsSceneChanging = false;
			m_SceneChangedInTransition = false;
			m_PendingCommit = false;
		}

		return; // 遷移中は旧 or 新シーンの Update をここでは進めないなら return
	}

	// 2. 通常更新
	if (m_CurrentScene)
	{
		m_CurrentScene->Update(delta_time);

		// シーン側が「切り替えたい」と言ってきたら、ここでリクエストを投げる
		if (m_CurrentScene->GetChangeScene())
		{
			const std::string next = m_CurrentScene->GetNextSceneName();

			auto fade = std::make_unique<FadeTransition>(
				2.5f,  // 秒
				FadeTransition::Mode::FadeInOut
			);
			RequestChangeScene(next, std::move(fade));
		}
	}
}
void SceneManager::DrawWorld(void)
{
	// シーン描画
	if (m_CurrentScene)
	{
		m_CurrentScene->Draw();
	}
}

void SceneManager::DrawTransition(void)
{
	// 遷移演出描画（フェードなど）
	if (m_IsSceneChanging && m_Transition)
	{
		m_Transition->Draw();
	}
}

void SceneManager::DrawUI()
{
	// シーンUI描画
	if (m_CurrentScene)
	{
		m_CurrentScene->DrawUI();
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

void SceneManager::CommitSceneChange(void)
{
	if (!m_IsSceneChanging) { return; }
	if (!m_PendingCommit) { return; }

	// ここで初めて切り替える（Draw後）
	SwitchSceneCore(m_NextSceneName);

	// 新シーンのオブジェクト初期化キューを消化
	if (m_pObjectManager)
	{
		m_pObjectManager->FlushSpawnQueue();
		m_pObjectManager->FlushAwakeQueue();
		m_pObjectManager->FlushStartQueue();
	}

	if (m_Transition)
	{
		m_Transition->OnSceneChanged();
		m_SceneChangedInTransition = true;
	}

	// 演出なしの場合はここで遷移状態を終わらせる
	if (!m_Transition)
	{
		m_IsSceneChanging = false;
		m_SceneChangedInTransition = false;
	}

	m_PendingCommit = false;
}

#ifdef _DEBUG

void SceneManager::DebugImGui()
{
	if (!ImGui::Begin("Scene Manager"))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Current: %s", m_CurrentSceneName.c_str());
	ImGui::Text("Changing: %s", m_IsSceneChanging ? "true" : "false");

	const auto& names = SceneClassFactory::GetInstance().GetRegisteredSceneNames();
	if (names.empty())
	{
		ImGui::Text("No registered scenes.");
		ImGui::End();
		return;
	}

	// 現在シーンをデフォルト選択にする
	static std::string selected;
	if (selected.empty())
		selected = m_CurrentSceneName.empty() ? names[0] : m_CurrentSceneName;

	// 現在名が変わったら追従
	if (!m_CurrentSceneName.empty() && selected != m_CurrentSceneName && !m_IsSceneChanging)
	{
		// 「ユーザーが選んだまま保持したい」ならこの if は消してOK
	}

	if (ImGui::BeginCombo("Next Scene", selected.c_str()))
	{
		for (const auto& n : names)
		{
			const bool isSelected = (selected == n);
			if (ImGui::Selectable(n.c_str(), isSelected))
				selected = n;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// 演出なし切替（Draw後に Commit で実行）
	if (ImGui::Button("Change (No Transition)"))
	{
		if (!m_IsSceneChanging && !selected.empty() && selected != m_CurrentSceneName)
		{
			Time::GetInstance().SetTimeScale(1.0f);
			RequestChangeScene(selected, nullptr);
		}
	}

	ImGui::SameLine();

	// 同じシーンを作り直したい（リロード）
	if (ImGui::Button("Reload"))
	{
		if (!m_IsSceneChanging && !m_CurrentSceneName.empty())
		{
			RequestChangeScene(m_CurrentSceneName, nullptr);
		}
	}

	ImGui::End();
}
#endif
