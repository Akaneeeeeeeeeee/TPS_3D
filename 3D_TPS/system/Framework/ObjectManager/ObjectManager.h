#pragma once
#include "system/Framework/ObjectManager/SnowFlakeID.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include "system/Framework/Factory/GameObjectFactory.h"


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

	void FlushAwakeQueue(void);				// Awakeキューの消化
	void FlushStartQueue(void);				// Startキューの消化
	void FlushDestroyQueue(void);			// 削除キューの消化(ここでのみ破棄を行う)
	void RequestDestroy(GameObject* obj);	// オブジェクト破棄リクエスト

	void SetCurrentSceneName(const std::string& name) { m_CurrentSceneName = name; }
	const std::string& GetCurrentSceneName(void) const { return m_CurrentSceneName; }

	void DestroySceneObjects(const std::string& sceneName);

private:
	Snowflake m_IDGenerator;	//! ID生成用のSnowflakeインスタンス
	
	GameObjectFactory* m_ObjectFactory;
	std::string m_CurrentSceneName;		// 「今のシーン名」	

	std::vector<std::unique_ptr<GameObject>> m_pObjects;				//! オブジェクトのコンテナ(ここが所有権を持つ)
	std::unordered_map<Tag, std::vector<GameObject*>> m_ObjectsByTag;	//! タグごとにオブジェクトを管理するためのmap
	std::unordered_map<uint64_t, GameObject*> m_ObjectsByID;			//! IDごとにオブジェクトを管理するためのmap
	std::unordered_map<std::string, GameObject*> m_ObjectsByName;		//! 名前ごとにオブジェクトを管理するためのmap
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
	
	// 各コンテナに追加
	m_pObjects.push_back(std::move(obj));
	m_ObjectsByTag[_Tag].push_back(rawPtr);
	m_ObjectsByID[id] = rawPtr;
	m_ObjectsByName[_Name] = rawPtr;

	// 初期化キューに積む
	m_PendingAwake.push_back(rawPtr);

	return static_cast<T*>(rawPtr);
}


template <typename T>
	requires std::derived_from<T, GameObject>
inline T* ObjectManager::GetObjectByName(const std::string& name) const
{
	auto it = m_ObjectsByName.find(name);
	if (it == m_ObjectsByName.end()) { return nullptr; }

	return dynamic_cast<T*>(it->second);
}

template <typename T>
	requires std::derived_from<T, GameObject>
inline T* ObjectManager::GetObjectByID(const uint64_t id) const
{
	// オブジェクトを探索
	auto it = m_ObjectsByID.find(id);
	if (it == m_ObjectsByID.end()) { return nullptr; }

	// 型を確認しキャスト
	if constexpr (std::is_same_v<T, GameObject>)
	{
		return static_cast<T*>(it->second);		// 型変換不要ならstatic_cast
	}
	else {
		return dynamic_cast<T*>(it->second);	// 安全にキャスト
	}
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

	for (auto* obj : it->second) {
		if (auto casted = dynamic_cast<T*>(obj)) {
			result.push_back(casted);
		}
	}
	return result;
}