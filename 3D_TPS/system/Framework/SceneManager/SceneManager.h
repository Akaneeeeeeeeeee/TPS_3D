#pragma once
#include "system/noncopyable.h"
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/Scene/IScene.h"
#include "system/SceneClassFactory.h"

/**
 * @brief シーン管理クラス
 * オブジェクト管理はタグと名前を使うが、シーンの管理はシーンの名前(こちらで定義)のみで行う
 * 
 * シーン切り替え関数が必要
*/
class SceneManager : public NonCopyable
{
public:
	/**
	 * @brief コンストラクタ
	 * @param _D3d11 d3dの参照
	 * タイトルシーンはゲーム開始すぐに必要なのでコンストラクタで生成する
	*/
	SceneManager()
	{
		// シーン保持しているコンテナを空にする
		this->m_pScenes.clear();
		this->m_CurrentSceneName = "";
		this->IsQuit = false;
	};
	~SceneManager() {};

	void Init(void);		//! 初期化
	void Update(void);		//! 更新
	void Draw(void);		//! 描画
	void Uninit(void);		//! 終了

	void ChangeScene(SceneName _NextScene);	//! シーン切り替え

	void CreateSceneInstance(SceneName _NewScene);
	
	void DeleteScene(SceneName _SceneName);
	
	void SetObjectManager(ObjectManager* _pObjectManager) { m_pObjectManager = _pObjectManager; }	//! オブジェクト管理クラスへのポインタをセット

	bool GetIsQuit(void) const { return IsQuit; }	//! ゲーム終了フラグ取得
	void SetIsQuit(bool _Flg) { IsQuit = _Flg; }	//! ゲーム終了フラグ設定

private:
	std::unordered_map<std::string, std::unique_ptr<IScene>> m_pScenes;	//! シーン配列
	std::string m_CurrentSceneName;				//! 現在のシーン名
	ObjectManager* m_pObjectManager = nullptr;	//! オブジェクト管理クラスへのポインタ
	SceneClassFactory* m_pSceneFactory;			//! シーンファクトリへのポインタ
	bool IsQuit = false;
};

