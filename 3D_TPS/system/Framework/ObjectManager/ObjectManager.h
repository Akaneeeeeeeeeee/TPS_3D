#pragma once
#include <span>
#include "system/Framework/ObjectManager/SnowFlakeID.h"
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/GameObject.h"
//#include "system/Framework/Graphics/RenderManager.h"
#include "system/Framework/ShaderManager/ShaderManager.h"

/**
 * @brief オブジェクトを管理するクラス
*/
class ObjectManager
{
public:
	ObjectManager() : m_IDGenerator(0) {};
	~ObjectManager() {};

	/**
	 * @brief オブジェクト個別追加関数
	 * @tparam T オブジェクトの型
	 * 
	 * ID設定、タグ設定、名前設定を行ってコンテナに追加
	*/
	template <typename T>
	T* CreateObject(const std::string& _Name, const Tag& _Tag = Tag::None)
	{
		// SnowfrakeIDを付与
		uint64_t id = this->m_IDGenerator.next_id();
		// コンパイル時チェック
		static_assert(std::is_base_of_v<GameObject, T>, "このオブジェクトはObjectを継承していません");
		// オブジェクト生成
		auto obj = std::make_unique<T>(m_pComponentFactory, id, _Name, _Tag);
		// オブジェクトの生ポインタを取得
		GameObject* rawPtr = obj.get();
		// コンテナに追加
		m_pObjects.push_back(std::move(obj));
		// タグごとにオブジェクトを管理するためのmapに追加
		m_ObjectsByTag[_Tag].push_back(rawPtr);
		// IDごとにオブジェクトを管理するためのmapに追加
		m_ObjectsByID[id] = rawPtr;
		// 名前ごとにオブジェクトを管理するためのmapに追加
		m_ObjectsByName[_Name] = rawPtr;

		return rawPtr;
	}

	// IDからオブジェクトを取得
	template <typename T = GameObject>
	T* GetObjectByID(uint64_t id) const
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
	template <typename T = GameObject>
	std::vector<T*> GetObjectsByTag(Tag& tag) 
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

	// 名前からオブジェクトを取得
	template <typename T = GameObject>
	T* GetObjectByName(const std::string& name) const 
	{
		auto it = m_ObjectsByName.find(name);
		if (it == m_ObjectsByName.end()) { return nullptr; }

		return dynamic_cast<T*>(it->second);
	}

	// オブジェクト削除
	void DeleteObject(Tag _ObjTag);

	/**
	 * @brief タグ変更関数
	 * @param _tag 
	 * @param _name 
	 * @param _newTag 
	 * @return 
	*/
	bool ChangeTag(uint64_t _id, const Tag& _newTag);

	void Init(ComponentFactory* _factory);
	void Update(void);
	void Draw(void);
	void Uninit(void);

	//! DI
	void SetComponentFactory(ComponentFactory* _factory) { m_pComponentFactory = _factory; }
	void SetRenderManager(RenderManager* _renderManager) { m_pRenderManager = _renderManager; }

private:
	Snowflake m_IDGenerator;	//! ID生成用のSnowflakeインスタンス
	
	ComponentFactory* m_pComponentFactory = nullptr;	//! コンポーネントファクトリーへのポインタ
	RenderManager* m_pRenderManager = nullptr;			//! レンダーマネージャーへのポインタ
	
	std::vector<std::unique_ptr<GameObject>> m_pObjects;					//! オブジェクトのコンテナ(ここが所有権を持つ)
	std::unordered_map<Tag, std::vector<GameObject*>> m_ObjectsByTag;	//! タグごとにオブジェクトを管理するためのmap
	std::unordered_map<uint64_t, GameObject*> m_ObjectsByID;			//! IDごとにオブジェクトを管理するためのmap
	std::unordered_map<std::string, GameObject*> m_ObjectsByName;		//! 名前ごとにオブジェクトを管理するためのmap
};

