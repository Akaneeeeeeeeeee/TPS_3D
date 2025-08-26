#pragma once
#include "system/Framework/ObjectManager/ObjectManager.h"

enum class SceneName {
	NONE = -1,		// シーンなし
	TITLE,
	GAME,
	RESULT,

	SCENE_MAX
};

/**
 * @brief シーンの抽象クラス
 *
 * 全てのシーンがこれを継承するのでポリモーフィズムでシーンを管理できる
 */
class IScene
{
public:
	virtual ~IScene() {};

	virtual void Init(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;
	virtual void Uninit(void) = 0;

	virtual void SetChangeScene(bool _Flg) { ChangeScene = _Flg; }
	virtual bool GetChangeScene(void) const { return ChangeScene; };
	virtual SceneName GetSceneName(void) const { return m_SceneName; }
	virtual SceneName GetRequestSceneName(void) const { return m_NextSceneName; }

	virtual void SetObjectManager(ObjectManager* _pObjectMgr) { m_pObjectManager = _pObjectMgr; }

protected:
	IScene() {};
	ObjectManager* m_pObjectManager = nullptr;		// オブジェクト管理クラスへのポインタ
	SceneName m_SceneName = SceneName::NONE;		// シーン名
	SceneName m_NextSceneName = SceneName::NONE;	// 次のシーン名
	bool ChangeScene = false;						// シーン切り替えフラグ
};

