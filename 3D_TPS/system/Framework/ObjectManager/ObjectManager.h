#pragma once
#include "system/Framework/ObjectManager/SnowFlakeID.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/EngineSystem/EngineSystem.h"
#include "system/Framework/Factory/GameObjectFactory.h"
#include "system/Framework/Game/GameResult.h"

/**
 * @brief オブジェクトを管理するクラス
*/
class ObjectManager
{
public:
	explicit ObjectManager()
		: m_ObjectFactory(nullptr), m_IDGenerator(0) {};
	~ObjectManager() = default;

	/**
	 * @brief オブジェクト個別追加関数
	 * @tparam T オブジェクトの型
	 * 
	 * ID設定、タグ設定、名前設定を行ってコンテナに追加
	*/
	template <typename T, typename ...Args>
		requires std::derived_from<T, GameObject>
	T* Instantiate(const std::string& _Name, const Tag _Tag = Tag::None, Args&&... args);

	// IDからオブジェクトを取得
	template <typename T>
		requires std::derived_from<T, GameObject>
	T* GetObjectByID(const uint64_t id) const;

	// 指定タグのオブジェクトを取得
	template <typename T>
		requires std::derived_from<T, GameObject>
	std::vector<T*> GetObjectsByTag(const Tag tag) const;

	template <typename T>
		requires std::derived_from<T, GameObject>
	T* GetObjectByTag(const Tag tag) const
	{
		auto objs = GetObjectsByTag<T>(tag);
		if (objs.empty()) { return nullptr; }
		return objs.front();
	}

	// 名前からオブジェクトを取得
	template <typename T>
		requires std::derived_from<T, GameObject>
	T* GetObjectByName(const std::string& name) const;

	// オブジェクト削除
	void DeleteObject(const Tag _ObjTag);
	void DeleteObject(const uint64_t _id);
	void DeleteObject(const std::string& _name);

	/**
	 * @brief タグ変更関数
	*/
	bool ChangeTag(const uint64_t _id, const Tag _newTag);

	void Init(GameObjectFactory* factory);
	void Update(const float deltatime);
	void Draw(void) const;
	void Uninit(void);

	void FlushSpawnQueue(void);				// 生成キューの消化
	void FlushAwakeQueue(void);				// Awakeキューの消化
	void FlushStartQueue(void);				// Startキューの消化
	void FlushDestroyQueue(void);			// 削除キューの消化(ここでのみ破棄を行う)
	void RequestDestroy(GameObject* obj);	// オブジェクト破棄リクエスト

	void SetCurrentSceneName(const std::string& name) { m_CurrentSceneName = name; }
	const std::string& GetCurrentSceneName(void) const { return m_CurrentSceneName; }

	void DestroySceneObjects(const std::string& sceneName);

	void SetGameResult(ResultType t) { m_Result.type = t; }
	ResultType GetGameResult() const { return m_Result.type; }
	void ClearGameResult() { m_Result = {}; }

private:
	Snowflake m_IDGenerator;	//! ID生成用のSnowflakeインスタンス

	GameResult m_Result{};

	GameObjectFactory* m_ObjectFactory;
	std::string m_CurrentSceneName;		// 「今のシーン名」

	std::vector<std::unique_ptr<GameObject>> m_pObjects;				//! オブジェクトのコンテナ(ここが所有権を持つ)
	std::unordered_map<Tag, std::vector<GameObject*>> m_ObjectsByTag;	//! タグごとにオブジェクトを管理するためのmap
	std::unordered_map<uint64_t, GameObject*> m_ObjectsByID;			//! IDごとにオブジェクトを管理するためのmap
	std::unordered_map<std::string, GameObject*> m_ObjectsByName;		//! 名前ごとにオブジェクトを管理するためのmap
	std::vector<std::unique_ptr<GameObject>> m_PendingSpawn;			//! 生成待ちオブジェクトのコンテナ
	std::vector<GameObject*> m_PendingAwake;							//! Awake待ちオブジェクトのコンテナ
	std::vector<GameObject*> m_PendingStart;							//! Start待ちオブジェクトのコンテナ
};


template <typename T, typename ...Args>
	requires std::derived_from<T, GameObject>
inline T* ObjectManager::Instantiate(const std::string& _Name, const Tag _Tag, Args&&... args)
{
	// SnowfrakeIDを付与
	uint64_t id = this->m_IDGenerator.next_id();
	
	// ファクトリ経由で生成
	auto obj = m_ObjectFactory->Create<T>(id, _Name, _Tag, std::forward<Args>(args)...);
	
	// オブジェクトの生ポインタを取得
	GameObject* rawPtr = obj.get();
	
	// 所有シーンと寿命を設定
	rawPtr->SetOwnerScene(m_CurrentSceneName);
	rawPtr->SetLifetime(GameObject::Lifetime::Scene); // デフォルトはシーン限定
	rawPtr->SetObjectManager(this); // オブジェクトマネージャーをセット
	
	// まだ世界に登録せず、生成しただけ
	m_PendingSpawn.push_back(std::move(obj));

	return static_cast<T*>(rawPtr);
}


template <typename T>
	requires std::derived_from<T, GameObject>
inline T* ObjectManager::GetObjectByName(const std::string& name) const
{
	auto it = m_ObjectsByName.find(name);
	if (it == m_ObjectsByName.end()) { return nullptr; }

	GameObject* obj = it->second;
	if (!obj) { return nullptr; }

	if constexpr (std::is_same_v<T, GameObject>)
	{
		return obj;
	}

	if (!obj->IsObjA<T>()) { return nullptr; }

	return static_cast<T*>(obj);
}

template <typename T>
	requires std::derived_from<T, GameObject>
inline T* ObjectManager::GetObjectByID(const uint64_t id) const
{
	// オブジェクトを探索
	auto it = m_ObjectsByID.find(id);
	if (it == m_ObjectsByID.end()) { return nullptr; }

	GameObject* obj = it->second;
	if (!obj) { return nullptr; }

	if constexpr (std::is_same_v<T, GameObject>)
	{
		return obj;
	}

	if (!obj->IsObjA<T>()) { return nullptr; }

	return static_cast<T*>(obj);
}

// 指定タグのオブジェクトを取得
template <typename T>
	requires std::derived_from<T, GameObject>
inline std::vector<T*> ObjectManager::GetObjectsByTag(const Tag tag) const
{
	std::vector<T*> result;

	auto it = m_ObjectsByTag.find(tag);
	if (it == m_ObjectsByTag.end()) { return result; }

	// メモリを再確保せずにサイズを設定
	result.reserve(it->second.size());

	for (auto* obj : it->second)
	{
		if (!obj) { continue; }

		// 型チェック
		if constexpr (std::is_same_v<T, GameObject>)
		{
			result.push_back(obj);
		}
		else
		{
			if (obj->IsObjA<T>())
			{
				result.push_back(static_cast<T*>(obj));
			}
		}
	}
	return result;
}