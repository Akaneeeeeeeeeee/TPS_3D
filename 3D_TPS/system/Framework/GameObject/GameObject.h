#pragma once
#include <typeindex>
#include "system/CVertexBuffer.h"
#include "system/CIndexBuffer.h"
#include "system/Framework/Component/Transform/Transform.h"
#include "system/Framework/Component/IComponent/IComponent.h"
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
	Item,			//! アイテム
	Light,			//! ライト
	UI,				//! UI
	Effect,			//! エフェクト
};

class GameObject {
public:
	GameObject() = delete;
	GameObject(uint64_t id, const std::string& name = "", const Tag& tag = Tag::None);

	virtual ~GameObject();		//! デストラクタ

	virtual void Init(void);
	virtual void Update(uint64_t deltatime);
	virtual void Draw(uint64_t deltatime);
	virtual void Uninit(void);

	// 姿勢情報のゲッター/セッター
	virtual Transform GetTransform(void) const { return m_Transform; }
	virtual const Transform& GetTransformRef(void) const { return m_Transform; }
	virtual void SetTransform(const Transform& transform) { m_Transform = transform; }
	virtual Vector3 GetPosition(void) const;
	virtual const Vector3& GetPositionRef(void) const { return m_Transform.GetPositionRef(); }
	virtual void SetPosition(const Vector3& position);
	virtual Quaternion GetRotation(void) const;
	virtual const Quaternion& GetRotationRef(void) const { return m_Transform.GetRotationRef(); }
	virtual void SetRotation(const Quaternion& rotation);
	virtual Vector3 GetScale(void) const;
	virtual const Vector3& GetScaleRef(void) const { return m_Transform.GetScaleRef(); }
	virtual void SetScale(const Vector3& scale);

	// Transform関連は直接委譲
	virtual Matrix4x4 GetWorldMatrix() const { return m_Transform.GetWorldMatrix(); }

	virtual Tag& GetTag(void) { return m_Tag; }
	virtual void SetTag(const Tag& tag) { m_Tag = tag; }	// これはObjectMangerからのみ呼び出す
	virtual uint64_t GetID(void) const { return m_ID; }
	virtual std::string GetName(void) const { return m_Name; }

protected:
	// SRT情報（姿勢情報）
	Transform m_Transform{};
	//! 描画の為の情報（見た目に関わる部分）
	//Shader m_Shader; // シェーダー

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
};
