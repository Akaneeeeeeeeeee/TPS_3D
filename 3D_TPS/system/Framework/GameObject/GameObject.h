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
	explicit GameObject(EngineContext& context,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::None,
		const Transform& transform = Transform::One());

	explicit GameObject(EngineContext& context,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::None,
		const Vector3& pos = Vector3::Zero,
		const Quaternion& rot = Quaternion::Identity,
		const Vector3& scale = Vector3::One);

	virtual ~GameObject() = default;		//! デストラクタ

	virtual ~GameObject() = default;		//! デストラクタ

	virtual void Init(void);
	virtual void Update(const float deltatime);
	virtual void Draw(void) const;
	virtual void Uninit(void);
	virtual EngineContext& GetContext(void) { return m_Context; }

	//////////////////////////////////////////
	//			コンポーネントの取り外し			//
	//////////////////////////////////////////
	template<typename T, typename ...Args>
		requires std::derived_from<T, IComponent>
	T* AddComponent(const std::string& name, Args&&... args);

	// 型指定でのコンポーネント取得
	template <typename T>
		requires std::derived_from<T, IComponent>
	T* GetComponent(void);
	template <typename T>
		requires std::derived_from<T, IComponent>
	T* GetComponent(const std::string& name);

	// GameObject に全部取り出すヘルパ
	template <typename T>
		requires std::derived_from<T, IComponent>
	void GetComponents(std::vector<T*>& components);

	template <typename T>
		requires std::derived_from<T, IComponent>
	bool RemoveComponent(T* component);

	virtual IComponent* GetComponent(const std::string& name) const;
	virtual void RemoveComponent(const std::string& name);

	//////////////////////////////////////////////
	//			姿勢情報のゲッター/セッター			//
	//////////////////////////////////////////////
	virtual Transform GetTransform(void) const { return m_Transform; }
	virtual Transform& TransformRef(void) { return m_Transform; }
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
	virtual bool IsActive(void) const { return m_IsActive; }
	virtual void SetActive(const bool isActive) { m_IsActive = isActive; }
	virtual bool IsDestroy(void) const { return m_IsDestroy; }
	virtual void Destroy(void) { m_IsDestroy = true; }
	virtual void SetDestroy(const bool isDestroy) { m_IsDestroy = isDestroy; }

private:
	// コンポーネントの生成
	template<typename T, typename ...Args>
		requires std::derived_from<T, IComponent>
	std::unique_ptr<T> CreateComponent(Args... arg);

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


template<typename T, typename ...Args>
	requires std::derived_from<T, IComponent>
inline T* GameObject::AddComponent(const std::string& name, Args&&... args)
{
	// 同名のコンポーネントが存在するなら追加しない
	if (m_Components.find(name) != m_Components.end()) { return nullptr; }

	// ユニークポインタ生成
	auto component = this->CreateComponent<T>(std::forward<Args>(args)...);

	// 所有者と初期化
	component->SetOwner(this);

	T* ptr = component.get();
	m_Components[name] = std::move(component);
	m_Components[name]->Attach(m_Context);
	return ptr;
}

template <typename T>
	requires std::derived_from<T, IComponent>
inline bool GameObject::RemoveComponent(T* component)
{
	if (!component) { return false; }

	// コンポーネント探索
	auto it = std::find_if(m_Components.begin(), m_Components.end(),
		[&](const auto& pair) { return pair.second.get() == component; });
	if (it == m_Components.end()) { return false; }

	// 取り外し→終了処理
	it->second->Uninit();
	it->second->Detach(m_Context);
	m_Components.erase(it);
	return true;
}

template <typename T>
	requires std::derived_from<T, IComponent>
inline T* GameObject::GetComponent(void)
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
	requires std::derived_from<T, IComponent>
inline T* GameObject::GetComponent(const std::string& _name)
{
	// コンポーネント探索
	for (auto& comp : m_Components)
	{
		if (comp.first == _name)
		{
			return static_cast<T*>(comp.second.get());
		}
	}
	return nullptr;
}

// GameObject に全部取り出すヘルパ
template <typename T>
	requires std::derived_from<T, IComponent>
inline void GameObject::GetComponents(std::vector<T*>& outcomponents)
{
	outcomponents.clear();
	for (auto& [name, comp] : m_Components)
	{
		if (auto p = static_cast<T*>(comp.get())) // 同一ヒエラルキーなのでOK
		{
			outcomponents.push_back(p);
		}
	}
}

template<typename T, typename ...Args>
	requires std::derived_from<T, IComponent>
inline std::unique_ptr<T> GameObject::CreateComponent(Args... arg)
{
	std::unique_ptr<T> component = std::make_unique<T>(arg...);
	return std::move(component);
}