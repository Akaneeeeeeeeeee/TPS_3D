#include "GameObject.h"

GameObject::GameObject(uint64_t id, const std::string& name, const Tag& tag)
	:m_ID(id), m_Name(name), m_Tag(tag),
	m_Transform(
		Vector3(0.0f, 0.0f, 0.0f), Quaternion(0.0f, 0.0f, 0.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f))
{

}

/**
 * @brief デストラクタ
*/
GameObject::~GameObject() {

}

void GameObject::Init(void)
{
	// コンポーネントの初期化
	//m_Components.clear();
}

void GameObject::Update(uint64_t deltatime)
{
	// コンポーネントの更新
	/*for(auto& component : m_Components) {
		component.second->Update();
	}*/
}

/// <summary>
/// レンダラー系コンポーネントを保持していれば描画する
/// →毎フレームコンポーネントを捜索するのは非効率的なのでフラグを持たせるべきかも
/// </summary>
/// <param name=""></param>
void GameObject::Draw(uint64_t deltatime)
{
	// レンダラー系コンポーネントを保持していれば描画する
	/*for (auto& component : m_Components) 
	{
		if (auto renderer = dynamic_cast<IRenderer*>(component.second.get())) 
		{
			renderer->Render();
		}
	}*/
}

void GameObject::Uninit(void)
{
	// コンポーネントの終了処理
	/*for (auto& component : m_Components) {
		component.second->Uninit();
	}
	m_Components.clear();*/
}

// Positionゲッター
Vector3 GameObject::GetPosition(void) const {
	return m_Transform.GetPosition();
}

// Positionセッター
void GameObject::SetPosition(const Vector3& _pos) {
	this->m_Transform.SetPosition(_pos);
}

// Rotationゲッター
Quaternion GameObject::GetRotation(void) const {
	return m_Transform.GetRotation();
}

// Rotationセッター
void GameObject::SetRotation(const Quaternion& _rot) {
	this->m_Transform.SetRotation(_rot);
}

// Scaleゲッター
Vector3 GameObject::GetScale(void) const {
	return m_Transform.GetScale();
}

// Scaleセッター
void GameObject::SetScale(const Vector3& _scale) {
	this->m_Transform.SetScale(_scale);
}
