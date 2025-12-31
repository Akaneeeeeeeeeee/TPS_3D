#include "GameObject.h"

GameObject::GameObject(ComponentFactory* factory,
	const uint64_t id,
	const std::string& name,
	const Tag tag,
	const Transform& transform)
	:m_pComponentFactory(factory),
	m_ID(id), m_Name(name), m_Tag(tag),
	m_Transform(transform)
{

}

GameObject::GameObject(ComponentFactory* factory,
	const uint64_t id,
	const std::string& name,
	const Tag tag,
	const Vector3& pos,
	const Quaternion& rot,
	const Vector3& scale)
	: GameObject(factory, id, name, tag, Transform(pos, rot, scale))
{

}

void GameObject::RemoveComponent(const std::string& name)
{
	// コンポーネント探索
	auto it = m_Components.find(name);
	if (it == m_Components.end()) { return; }

	// 見つかったら削除
	IComponent* comp = it->second.get();

	// 初期化キューからも除外
	m_InitializeQueue.erase(
		std::remove(m_InitializeQueue.begin(), m_InitializeQueue.end(), comp),
		m_InitializeQueue.end()
	);

	comp->Uninit();
	comp->Detach();
	m_Components.erase(it);
}

// 保留中の初期化コンポーネントを初期化する
void GameObject::FlushInitializeQueue(void)
{
	if (m_InitializeQueue.empty()) { return; }

	// 保留中の初期化コンポーネントを初期化する
	for (auto& component : m_InitializeQueue)
	{
		if (!component) { continue; }

		// 消す予定なら初期化しない
		if (component->IsDestroyRequested()) { continue; }

		component->Init();
	}
	m_InitializeQueue.clear();
}

// 破棄予約されたコンポーネントを破棄する
void GameObject::FlushDestroyComponents(void)
{
	// 破棄予約されたコンポーネントを破棄する
	for (auto it = m_Components.begin(); it != m_Components.end(); )
	{
		if (it->second->IsDestroyRequested())
		{
			it->second->Uninit();
			it->second->Detach();
			it = m_Components.erase(it);
		}
		else
		{
			++it;
		}
	}
	// 初期化キュー側にも残っている可能性があるので掃除
	m_InitializeQueue.erase(
		std::remove_if(
			m_InitializeQueue.begin(), m_InitializeQueue.end(),
			[](IComponent* c) { return c && c->IsDestroyRequested(); }
		),
		m_InitializeQueue.end()
	);
}

void GameObject::AwakeOnce(void)
{
	if (m_AwakeDone) { return; }
	m_AwakeDone = true;

	Awake();                 // 派生の処理
	FlushInitializeQueue();   // Awake中にAddComponentした分を必ずInitする
}

void GameObject::StartOnce(void)
{
	// Awake前にStartさせない
	if (!m_AwakeDone) { return; }
	if (m_StartDone) { return; }
	m_StartDone = true;

	Start();                 // 派生の処理
}

void GameObject::BaseUpdate(const float deltatime)
{
	// 未初期化のコンポーネントがあれば初期化する
	FlushInitializeQueue();

	// 派生先のUpdate呼び出し
	Update(deltatime);

	// コンポーネントの更新
	for (auto& component : m_Components)
	{
		if (component.second->GetIsValid())
		{
			component.second->Update(deltatime);
		}
	}

	// 破棄予約されたコンポーネントを破棄する
	FlushDestroyComponents();
}

void GameObject::Update(const float deltatime)
{
}

void GameObject::BaseLateUpdate(const float deltatime)
{
	// Update中にAddComponentされたものは「このフレームのLateUpdateに入れる」
	FlushInitializeQueue();

	LateUpdate(deltatime);

	for (auto& [name, comp] : m_Components)
	{
		if (comp && comp->GetIsValid())
		{
			comp->LateUpdate(deltatime);
		}
	}

	// LateUpdate中にDestroyされたコンポーネントをここで掃除
	FlushDestroyComponents();
}

void GameObject::LateUpdate(const float deltatime)
{
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

void GameObject::BaseDraw(void) const
{
	Draw();
}

void GameObject::Uninit(void)
{
}

void GameObject::BaseUninit(void)
{
	// 派生先の終了処理
	Uninit();

	// コンポーネントの終了処理→取り外し
	for (auto& component : m_Components) {
		component.second->Uninit();
		component.second->Detach();
	}
	m_Components.clear();
	m_InitializeQueue.clear();
}