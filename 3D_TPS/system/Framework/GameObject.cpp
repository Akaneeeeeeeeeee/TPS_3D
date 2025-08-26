#include "GameObject.h"
#include "Src/Framework/Component/Renderer/SpriteRenderer/SpriteRenderer.h"

GameObject::GameObject() {

}

/**
 * @brief コンストラクタ
 * @param cam カメラのポインタ
*/
GameObject::GameObject(Camera* cam) :m_Camera(cam) {

}

/**
 * @brief デストラクタ
*/
GameObject::~GameObject() {

}

void GameObject::Init(void)
{
	// コンポーネントの初期化
	m_Components.clear();
}

void GameObject::Update(void)
{
	// コンポーネントの更新
	for(auto& component : m_Components) {
		component.second->Update();
	}
}

/// <summary>
/// レンダラー系コンポーネントを保持していれば描画する
/// →毎フレームコンポーネントを捜索するのは非効率的なのでフラグを持たせるべきかも
/// </summary>
/// <param name=""></param>
void GameObject::Draw(void)
{
	// レンダラー系コンポーネントを保持していれば描画する
	for (auto& component : m_Components) 
	{
		if (auto renderer = dynamic_cast<IRenderer*>(component.second.get())) 
		{
			renderer->Render();
		}
	}
}

void GameObject::Uninit(void)
{
	// コンポーネントの終了処理
	for (auto& component : m_Components) {
		component.second->Uninit();
	}
	m_Components.clear();
}

// Positionゲッター
const Vector3& GameObject::GetPosition(void) const {
	return m_Transform.GetPosition();
}

// Positionセッター
void GameObject::SetPosition(const Vector3& _pos) {
	this->m_Transform.SetPosition(_pos);
}

// Rotationゲッター
const Quaternion& GameObject::GetRotation(void) const {
	return m_Transform.GetRotation();
}

// Rotationセッター
void GameObject::SetRotation(const Quaternion& _rot) {
	this->m_Transform.SetRotation(_rot);
}

// Scaleゲッター
const Vector3& GameObject::GetScale(void) const {
	return m_Transform.GetScale();
}

// Scaleセッター
void GameObject::SetScale(const Vector3& _scale) {
	this->m_Transform.SetScale(_scale);
}

// ワールド行列を取得
DirectX::SimpleMath::Matrix GameObject::GetWorldMatrix(const DirectX::SimpleMath::Matrix& _parentmatrix) {
	return this->m_Transform.GetWorldMatrix(_parentmatrix);
}


//今のところ記述内容は無し

