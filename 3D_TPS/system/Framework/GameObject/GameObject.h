#pragma once
#include "system/Framework/Component/Transform/Transform.h"
#include "system/Framework/Component/IComponent/IComponent.h"
#include "system/Framework/EngineContext/EngineContext.h"
//#include "system/Framework/Component/Renderer/SpriteRenderer/SpriteRenderer.h"
//#include "system/Framework/Component/ComponentFactory/ComponentFactory.h"
//#include "system/Framework/AssetManager/AssetManager.h"
//#include "system/Framework/ShaderManager/ShaderManager.h"


//! オブジェクト管理用タグ
enum class Tag {
	None,			//! タグなし
	Player,			//! プレイヤー
	Field,			//! フィールド
	Skydome,		//! スカイドーム
	Enemy,			//! 敵
	Object,			//! オブジェクト
	Item,			//! アイテム
	Light,			//! ライト
	UI,				//! UI
	Effect,			//! エフェクト
};

/*
* @brief	ゲームオブジェクトクラス
* @detail	ゲーム内の全てのオブジェクトはこのクラスを継承して作成する
* @remark	オブジェクトはコンポーネントを管理する
* 
* @author	赤根和樹
* @date		2025/10/10(最終更新)
*/
class GameObject {
public:
	GameObject() = delete;
	GameObject(EngineContext& context,
		const uint64_t id,
		const std::string& name = "",
		const Tag& tag = Tag::None,
		const Transform& transform = Transform::One())
		: m_Context(context),
		m_ID(id),
		m_Name(name),
		m_Tag(tag),
		m_Transform(transform)
	{
	}

	GameObject(EngineContext& context,
		const uint64_t id,
		const std::string& name = "",
		const Tag& tag = Tag::None,
		const Vector3& pos = Vector3::Zero,
		const Quaternion& rot = Quaternion::Identity,
		const Vector3& scale = Vector3::One)
		: GameObject(context, id, name, tag, Transform(pos, rot, scale))
	{
	}
	virtual ~GameObject();		//! デストラクタ

	virtual void Init(void);
	virtual void Update(const uint64_t deltatime);
	virtual void Draw(uint64_t deltatime) const;
	virtual void Uninit(void);

	//////////////////////////////////////////
	//			コンポーネントの取り外し			//
	//////////////////////////////////////////
	template<typename T, typename ...Args>
	T* AddComponent(const std::string& name, Args&&... args)
	{
		// 継承チェック
		static_assert(std::is_base_of<IComponent, T>::value, "TはIComponentを継承していません");

		// 同名のコンポーネントが存在するなら追加しない
		if (m_Components.find(name) != m_Components.end()) { return nullptr; }

		// ユニークポインタ生成
		auto component = this->CreateComponent<T>(std::forward<Args>(args)...);

		// 所有者と初期化
		component->SetOwner(*this);

		T* ptr = component.get();
		m_Components[name] = std::move(component);
		return ptr;
	}

	// 型指定でのコンポーネント取得
	template <typename T>
	T* GetComponent(void)
	{
		// コンポーネント探索
		for (auto& [name, comp] : m_Components) 
		{
			if (auto ptr = dynamic_cast<T*>(comp.get()))
			{
				return ptr;
			}
		}
		return nullptr;
	}

	template <typename T>
	bool RemoveComponent(T* component)
	{
		if (!component) { return false; }

		// コンポーネント探索
		auto it = std::find_if(m_Components.begin(), m_Components.end(),
			[&](const std::unique_ptr<IComponent>& p) { return p.get() == component; });
		if (it == m_Components.end()) { return false; }

		// 取り外し→終了処理
		(*it)->Uninit();
		(*it)->Detach(m_Context);
		// インデックスからも外す
		auto& vec = m_Components[typeid(T)];
		vec.erase(std::remove(vec.begin(), vec.end(), component), vec.end());
		m_Components.erase(it);
		return true;
	}

	virtual IComponent* GetComponent(const std::string& name) const;

	//////////////////////////////////////////////
	//			姿勢情報のゲッター/セッター			//
	//////////////////////////////////////////////
	virtual Transform GetTransform(void) const { return m_Transform; }
	virtual void SetTransform(const Transform& transform) { m_Transform = transform; }
	virtual Vector3 GetPosition(void) const { return m_Transform.GetPosition(); }
	virtual void SetPosition(const Vector3& position) { m_Transform.SetPosition(position); }
	virtual Quaternion GetRotation(void) const { return m_Transform.GetRotation(); }
	virtual void SetRotation(const Quaternion& rotation) { m_Transform.SetRotation(rotation); }
	virtual Vector3 GetScale(void) const { return m_Transform.GetScale(); }
	virtual void SetScale(const Vector3& scale) { m_Transform.SetScale(scale); }

	// Transform関連は直接委譲
	virtual Matrix4x4 GetWorldMatrix(void) const { return m_Transform.GetWorldMatrix(); }

	virtual Tag GetTag(void) const { return m_Tag; }
	virtual void SetTag(const Tag& tag) { m_Tag = tag; }	// これはObjectMangerからのみ呼び出す
	virtual uint64_t GetID(void) const { return m_ID; }
	virtual std::string GetName(void) const { return m_Name; }

private:
	// コンポーネントの生成
	template<typename T, typename ...Args>
	std::unique_ptr<T> CreateComponent(Args... arg)
	{
		static_assert(std::is_base_of<IComponent, T>::value, "TはComponentを継承していません");
		std::unique_ptr<T> component = std::make_unique<T>(arg...);
		return std::move(component);
	}

protected:
	// SRT情報（姿勢情報）
	Transform m_Transform;
	//! 描画の為の情報（見た目に関わる部分）
	//Shader m_Shader; // シェーダー

	// コンテキストへの参照
	EngineContext& m_Context;

	//! 一意のID
	uint64_t m_ID = 0;

	//! タグ（オブジェクトの種類を示す）
	Tag m_Tag = Tag::None;

	//! オブジェクトの名前
	std::string m_Name;

	//! シェーダーマネージャー
	//ShaderManager* m_pShaderManager;

	//! オブジェクトがアクティブかどうか（trueなら更新・描画する）
	bool m_IsActive = true;

	// オブジェクトが削除されているかどうか（trueなら削除済み）
	bool m_IsDestroy = false;

	//! コンポーネントのマップ(コンポーネントが多数になる場合はunordered_mapとの併用も検討)
	std::unordered_map<std::string, std::unique_ptr<IComponent>> m_Components;
};
