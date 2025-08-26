#pragma once
#include <typeindex>
#include "system/Framework/Buffer/VertexBuffer.h"
#include "system/Framework/Buffer/IndexBuffer.h"
#include "system/Framework/Shader/Shader.h"
#include "system/Camera.h"
#include "system/Framework/Component/Transform/Transform.h"
#include "system/Framework/Component/IComponent/IComponent.h"
#include "system/Framework/Component/Renderer/SpriteRenderer/SpriteRenderer.h"
#include "system/Framework/Component/ComponentFactory/ComponentFactory.h"

//! オブジェクト管理用タグ
enum class Tag {
	None,			//! タグなし
	Player,			//! プレイヤー
	Enemy,			//! 敵
	Item,			//! アイテム
	Light,			//! ライト
	UI,				//! UI
	Effect,			//! エフェクト
};

class GameObject {
public:
	GameObject();
	GameObject(ComponentFactory* _factory, uint64_t id, const std::string& name = "", const Tag& tag = Tag::None)
		: m_pComponentFactory(_factory), m_ID(id), m_Name(name), m_Tag(tag),
		  m_Transform(Vector3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f)) {
		// カメラはnullptrで初期化
		m_Camera = nullptr;
	}

	GameObject(Camera* cam);	//! コンストラクタ
	virtual ~GameObject();		//! デストラクタ

	virtual void Init(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Uninit(void);

	// 姿勢情報のゲッター/セッター
	virtual Transform& GetTransform(void) { return m_Transform; };
	virtual void SetTransform(const Transform& transform) { m_Transform = transform; }
	virtual const Vector3& GetPosition(void) const;
	virtual void SetPosition(const Vector3& position);
	virtual const Quaternion& GetRotation(void) const;
	virtual void SetRotation(const Quaternion& rotation);
	virtual const Vector3& GetScale(void) const;
	virtual void SetScale(const Vector3& scale);

	DirectX::SimpleMath::Matrix GetWorldMatrix(const DirectX::SimpleMath::Matrix& _parentmatrix);


	virtual Tag& GetTag(void) { return m_Tag; }
	virtual void SetTag(const Tag& tag) { m_Tag = tag; }	// これはObjectMangerからのみ呼び出す

	//-----------------------------------
	//			コンポーネント管理
	//-----------------------------------
	template<typename T>
	T* AddComponent(void)
	{
		// ファクトリからコンポーネントを生成
		auto component = this->m_pComponentFactory->CreateComponent<T>();

		// 初期化
		component->Init();

		// 所有者をセット
		component->SetOwner(this);

		// 名前をキーにして保存
		m_Components[typeid(T)] = std::move(component);

		// 登録したコンポーネントのポインタを取得
		IComponent* rawPtr = m_Components[typeid(T)].get();

		// キャストして返す
		return static_cast<T*>(rawPtr);
	}

	template<typename T>
	T* GetComponent() {
		// コンポーネントを探す
		auto it = m_Components.find(typeid(T));

		if (it != m_Components.end()) {
			return static_cast<T*>(it->second.get());
		}
		return nullptr;
	}

protected:
	// SRT情報（姿勢情報）
	Transform m_Transform =
		Transform(Vector3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f)); // SRT情報を管理

	//! 描画の為の情報（見た目に関わる部分）
	//Shader m_Shader; // シェーダー

	//! カメラ
	Camera* m_Camera;

	//! 一意のID
	uint64_t m_ID = 0;

	//! タグ（オブジェクトの種類を示す）
	Tag m_Tag = Tag::None;

	//! オブジェクトの名前
	std::string m_Name;

	//! コンポーネントのファクトリー
	ComponentFactory* m_pComponentFactory = nullptr;

	//! コンポーネントのリスト(現状型情報で持ってるので同一コンポーネントの複数所持はできない)
	std::unordered_map<std::type_index, std::unique_ptr<IComponent>> m_Components;

	//! オブジェクトがアクティブかどうか（trueなら更新・描画する）
	bool m_IsActive = true;
};
