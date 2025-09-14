#pragma once
#include "system/Framework/ObjectManager/ObjectManager.h"
#include "system/Framework/Application/Entry/main.h"

class ObjectManager; // 前方宣言

/**
 * @brief シーンの抽象クラス
 *
 * 全てのシーンがこれを継承するのでポリモーフィズムでシーンを管理できる
 */
class IScene
{
public:
	virtual ~IScene() {};

	virtual void Init(ObjectManager* _pObjectMgr) = 0;
	virtual void Update(uint64_t deltatime) = 0;
	virtual void Draw(uint64_t deltatime) = 0;
	virtual void Uninit(void) = 0;

	virtual void SetChangeScene(bool _Flg) { ChangeScene = _Flg; }
	virtual bool GetChangeScene(void) const { return ChangeScene; };

	virtual const std::string& GetNextSceneName(void) { return m_NextSceneName; }
	virtual void SetNextSceneName(const std::string& name) { m_NextSceneName = name; }

protected:
	IScene() {};
	std::string m_NextSceneName;
	ObjectManager* m_pObjectManager;		// オブジェクト管理クラスへのポインタ
	bool ChangeScene = false;						// シーン切り替えフラグ
};

