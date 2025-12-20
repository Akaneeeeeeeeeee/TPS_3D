#pragma once
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/Application/Entry/main.h"

/**
 * @brief シーンの抽象クラス
 *
 * 全てのシーンがこれを継承するのでポリモーフィズムでシーンを管理できる
 */
class IScene
{
public:
	virtual ~IScene() = default;

	virtual void Init(ObjectManager* mgr) = 0;
	//virtual void Init(void) = 0;
	virtual void Update(const float deltatime) = 0;
	virtual void Draw(void) = 0;
	virtual void Uninit(void) = 0;

	virtual void SetChangeScene(bool _Flg) { ChangeScene = _Flg; }
	virtual bool GetChangeScene(void) const { return ChangeScene; };

	virtual const std::string& GetNextSceneName(void) { return m_NextSceneName; }
	virtual void SetNextSceneName(const std::string& name) { m_NextSceneName = name; }

protected:
	IScene() = default;
	ObjectManager* m_pObjectManager = nullptr;	// オブジェクト管理クラスへのポインタ
	//std::string m_SceneName;					// このシーンの名前
	std::string m_NextSceneName;				// 次シーンの名前
	bool ChangeScene = false;					// シーン切り替えフラグ
};

