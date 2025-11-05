#include "GameObject.h"

GameObject::GameObject(EngineContext& context,
	const uint64_t id,
	const std::string& name,
	const Tag tag,
	const Transform& transform)
	:m_Context(context),
	m_ID(id), m_Name(name), m_Tag(tag),
	m_Transform(transform)
{

}

GameObject::GameObject(EngineContext& context,
	const uint64_t id,
	const std::string& name,
	const Tag tag,
	const Vector3& pos,
	const Quaternion& rot,
	const Vector3& scale)
	: GameObject(context, id, name, tag, Transform(pos, rot, scale))
{

}

/*
* @brief	コンポーネント取得
* @detail	名前でコンポーネントを取得する
* @param	name	コンポーネント名
*/
IComponent* GameObject::GetComponent(const std::string& name) const
{
	// コンポーネント探索
	auto it = m_Components.find(name);
	// 見つかったらポインタを返す
	if (it != m_Components.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void GameObject::Init(void)
{
	// コンポーネントの初期化
	//m_Components.clear();
}

void GameObject::Update(const uint64_t deltatime)
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
void GameObject::Draw(void) const
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

