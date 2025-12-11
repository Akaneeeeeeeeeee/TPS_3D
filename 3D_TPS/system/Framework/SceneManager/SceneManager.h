#pragma once
#include "system/noncopyable.h"
#include <memory>
#include <string>

// 前方宣言
class IScene;
class ObjectManager;
class SceneTransition;

/*
* @brief	シーンマネージャークラス
* @detail	シーンの管理、シーン間の遷移演出を担当するクラス
* @author	赤根　和樹
* @date		2025/12/01(最終更新)
* @remark	シーンの生成にはファクトリを使用する
* @remarks	SceneManager は「現在シーン 1 つ」だけを持つ
*/
class SceneManager : public NonCopyable
{
public:
	SceneManager();
	~SceneManager();

	//  first_scene_name は "TitleScene" など、REGISTER_SCENE で登録した名前
	void Init(ObjectManager* object_manager, const std::string& first_scene_name);

	// 更新
	void Update(float delta_time);

	// 描画
	void Draw(void);

	// 終了
	void Uninit();

	// 遷移の入り口を 2 種類用意
	// 外部から「次のシーンに変えたい」ときに呼ぶ
	// transition が null なら演出なしで即切り替え
	// 遷移演出をフェード以外にしたくなれば、SceneTransitionの実装を変えるだけでよい。
	void RequestChangeScene(const std::string& next_scene_name, std::unique_ptr<SceneTransition> transition);

	// 演出なしで即切り替え
	void ChangeSceneImmediate(const std::string& next_scene_name);

	bool GetIsQuit(void) const { return m_IsQuit; }
	void SetIsQuit(bool value) { m_IsQuit = value; }

private:
	// Factory を使ってシーンを生成して Init まで行う
	// 重いロード処理を非同期にしたくなったらこの中だけに手を入れればよい
	std::unique_ptr<IScene> CreateScene(const std::string& scene_name);

	// 実際の切り替え本体（旧シーン Uninit → 新シーン Init）をまとめた関数
	// ・フラグはここでは触らない
	void SwitchSceneCore(const std::string& next_scene_name);
private:
	ObjectManager* m_pObjectManager = nullptr;

	std::unique_ptr<IScene> m_CurrentScene = nullptr;
	std::string m_CurrentSceneName;

	// 遷移制御
	std::string m_NextSceneName;
	std::unique_ptr<SceneTransition> m_Transition = nullptr;
	bool m_IsSceneChanging = false;

	bool m_IsQuit = false;
	bool m_SceneChangedInTransition = false; // 今のトランジション中で一度だけ切り替えたか
};