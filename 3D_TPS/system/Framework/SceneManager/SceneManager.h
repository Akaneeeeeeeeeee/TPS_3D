#pragma once
#include "system/noncopyable.h"
#include "system/SceneClassFactory.h"
#include "system/Framework/SceneManager/Transition/SceneTransition.h"
#include <future>

class IScene;
class ObjectManager;

struct Load3DInfo {
	std::string filename;
	std::string texdirectoryname;
	Load3DInfo(std::string p1, std::string p2) {
		filename = p1;
		texdirectoryname = p2;
	}
};

/**
 * @brief シーン管理クラス
 * オブジェクト管理はタグと名前を使うが、シーンの管理はシーンの名前(こちらで定義)のみで行う
 * 
 * シーン切り替え関数が必要
*/
class SceneManager : public NonCopyable
{
public:
	SceneManager()
	{
		// シーン保持しているコンテナを空にする
		this->m_pScenes.clear();
		this->m_CurrentSceneName = "";
		this->IsQuit = false;
	};
	~SceneManager() {};

	template <typename T>
	void AddScene(const std::string& name)
	{
		static_assert(std::is_base_of_v<IScene, T>, "T must be derived from IScene");
		if (m_pScenes.find(name) == m_pScenes.end()) {
			m_pScenes[name] = std::make_unique<T>();
			//m_pScenes[name]->Init(m_pObjectManager);
		}
	}

	void Init(ObjectManager* _pObjectMgr);		//! 初期化
	//void Init(SceneClassFactory* _factory, ObjectManager* _pObjectMgr);		//! 初期化
	void Update(uint64_t deltatime);			//! 更新
	void Draw(uint64_t deltatime);				//! 描画
	void Uninit(void);							//! 終了

	void SetCurrentScene(const std::string& name, std::unique_ptr<SceneTransition> transition = nullptr);
	void ChangeScene(const std::string& nextscenename);

	void SetSceneFactory(SceneClassFactory* factory);
	void SetObjectManager(ObjectManager* _pObjectManager);	//! オブジェクト管理クラスへのポインタをセット

	bool GetIsQuit(void) const { return IsQuit; }	//! ゲーム終了フラグ取得
	void SetIsQuit(bool _Flg) { IsQuit = _Flg; }	//! ゲーム終了フラグ設定

private:
	std::unordered_map<std::string, std::unique_ptr<IScene>> m_pScenes;	//! シーン配列
	std::string m_CurrentSceneName;				//! 現在のシーン名
	ObjectManager* m_pObjectManager;			//! オブジェクト管理クラスへのポインタ
	//SceneClassFactory* m_pSceneFactory;			//! シーンファクトリへのポインタ
	std::unique_ptr<SceneTransition> m_Transition;	//! シーン遷移演出オブジェクト
	bool IsSceneChanging = false;	//! シーン遷移中フラグ
	bool IsQuit = false;			//! ゲーム終了フラグ

	// 非同期シーンロード用
	//std::future<void> m_LoadingTask;
	//std::atomic<bool> m_LoadingDone{ false };
};

