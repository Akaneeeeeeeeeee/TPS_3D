#pragma once
#include "system/Framework/Component/Transform/Transform.h"
#include "system/Framework/Component/IComponent/IComponent.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include "system/Framework/Factory/ComponentFactory.h"
//#include "system/Framework/Component/Renderer/SpriteRenderer/SpriteRenderer.h"
//#include "system/Framework/Component/ComponentFactory/ComponentFactory.h"
//#include "system/Framework/AssetManager/AssetManager.h"
//#include "system/Framework/ShaderManager/ShaderManager.h"


//! オブジェクト管理用タグ
enum class Tag {
	None,			//! タグなし
	Camera,			//! カメラ
	Player,			//! プレイヤー
	Field,			//! フィールド
	Skydome,		//! スカイドーム
	Enemy,			//! 敵
	Object,			//! オブジェクト
	Item,			//! アイテム
	Goal,           //! ゴール
	Light,			//! ライト
	UI,				//! UI
	Effect,			//! エフェクト
};

inline uint64_t ToUserData(GameObject* go)
{
	return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(go));
}

inline GameObject* FromUserData(uint64_t data)
{
	return reinterpret_cast<GameObject*>(static_cast<uintptr_t>(data));
}

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
	explicit GameObject(ComponentFactory* factory,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::None,
		const Transform& transform = Transform::One());

	explicit GameObject(ComponentFactory* context,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::None,
		const Vector3& pos = Vector3::Zero,
		const Quaternion& rot = Quaternion::Identity,
		const Vector3& scale = Vector3::One);

	virtual ~GameObject() = default;		//! デストラクタ

	enum class Lifetime
	{
		Scene,   // シーン切り替え時に消す
		Global   // シーンをまたいで残す
	};

	void AwakeOnce(void);
	virtual void Awake(void);	// オブジェクト“単体”の準備。コンポーネント追加、メモリ確保、デフォルト値セットなど
	void StartOnce(void);
	virtual void Start(void);	// 他オブジェクト参照の解決、GetComponent / Find / 相互リンク、初回だけ行いたいセットアップ
	virtual void Update(const float deltatime);
	virtual void Draw(void) const;
	virtual void Uninit(void);

	// 当たり判定イベント
	virtual void OnCollisionEnter(GameObject& other) {}
	virtual void OnCollisionStay(GameObject& other) {}
	virtual void OnCollisionExit(GameObject& other) {}

	virtual void OnTriggerEnter(GameObject& other) {}
	virtual void OnTriggerStay(GameObject& other) {}
	virtual void OnTriggerExit(GameObject& other) {}

	virtual void OnCollisionCharacterEnter(GameObject& other) {}

	void SetLifetime(Lifetime lt) { m_Lifetime = lt; }
	Lifetime GetLifetime() const { return m_Lifetime; }

	void SetOwnerScene(const std::string& name) { m_OwnerSceneName = name; }
	const std::string& GetOwnerScene() const { return m_OwnerSceneName; }

	//////////////////////////////////////////
	//			コンポーネントの取り外し		//
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

	void RemoveComponent(const std::string& name);
	void FlushInitializeQueue(void);
	void FlushDestroyComponents(void);

	//////////////////////////////////////////////
	//			姿勢情報のゲッター/セッター			//
	//////////////////////////////////////////////
	Transform GetTransform(void) const { return m_Transform; }
	Transform& TransformRef(void) { return m_Transform; }
	void SetTransform(const Transform& transform) { m_Transform = transform; }
	Vector3 GetPosition(void) const { return m_Transform.GetPosition(); }
	void SetPosition(const Vector3& position) { m_Transform.SetPosition(position); }
	Quaternion GetRotation(void) const { return m_Transform.GetRotation(); }
	void SetRotation(const Quaternion& rotation) { m_Transform.SetRotation(rotation); }
	Vector3 GetScale(void) const { return m_Transform.GetScale(); }
	void SetScale(const Vector3& scale) { m_Transform.SetScale(scale); }
	Vector3 GetForward(void) const { return m_Transform.GetForward(); }

	// Transform関連は直接委譲
	Matrix4x4 GetWorldMatrix(void) const { return m_Transform.GetWorldMatrix(); }

	Tag GetTag(void) const { return m_Tag; }
	void SetTag(const Tag& tag) { m_Tag = tag; }	// これはObjectMangerからのみ呼び出す
	uint64_t GetID(void) const { return m_ID; }
	std::string GetName(void) const { return m_Name; }
	bool IsActive(void) const { return m_IsActive; }
	void SetActive(const bool isActive) { m_IsActive = isActive; }
	bool IsDestroy(void) const { return m_IsDestroy; }
	void Destroy(void) { m_IsDestroy = true; }
	void SetDestroy(const bool isDestroy) { m_IsDestroy = isDestroy; }

protected:
	// SRT情報（姿勢情報）
	Transform m_Transform;
	//! 描画の為の情報（見た目に関わる部分）
	//Shader m_Shader; // シェーダー

	ComponentFactory* m_pComponentFactory = nullptr;

	// コンテキストへの参照
	//EngineContext& m_Context;

	//! 一意のID
	uint64_t m_ID = 0;

	//! タグ（オブジェクトの種類を示す）
	Tag m_Tag = Tag::None;

	// デフォルトは「シーン限定」
	Lifetime    m_Lifetime = Lifetime::Scene;

	// どのシーンが作ったか
	std::string m_OwnerSceneName;

	//! オブジェクトがアクティブかどうか（trueなら更新・描画する）
	bool m_IsActive = true;

	// オブジェクトが削除されているかどうか（trueなら削除済み）
	bool m_IsDestroy = false;

	// 初期化済みフラグ
	bool m_AwakeDone = false;
	// 開始処理済みフラグ
	bool m_StartDone = false;
	
	//! オブジェクトの名前
	std::string m_Name;

	//! シェーダーマネージャー
	//ShaderManager* m_pShaderManager;

	//! コンポーネントのマップ(コンポーネントが多数になる場合はunordered_mapとの併用も検討)
	std::unordered_map<std::string, std::unique_ptr<IComponent>> m_Components;
	std::vector<IComponent*> m_InitializeQueue;		// 保留中の初期化コンポーネント
};


template<typename T, typename ...Args>
	requires std::derived_from<T, IComponent>
inline T* GameObject::AddComponent(const std::string& name, Args&&... args)
{
	// 同名のコンポーネントが存在するなら追加しない
	if (m_Components.find(name) != m_Components.end()) { return nullptr; }

	// 生成
	auto component = m_pComponentFactory->Create<T>(*this, std::forward<Args>(args)...);

	T* ptr = component.get();

	// 所有権は GameObject が持つ
	m_Components[name] = std::move(component);

	// 初期化待ちキューに積む
	m_InitializeQueue.push_back(ptr);
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

	// まだキューにいるかもしれないので一応取り除く
	m_InitializeQueue.erase(
		std::remove(m_InitializeQueue.begin(), m_InitializeQueue.end(), component),
		m_InitializeQueue.end()
	);

	// 終了処理
	it->second->Uninit();
	it->second->Detach();
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
