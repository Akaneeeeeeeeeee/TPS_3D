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

// 名前指定でのコンポーネント削除
void GameObject::RemoveComponent(const std::string& name)
{
	// コンポーネント探索
	auto it = m_Components.find(name);
	if (it == m_Components.end()) { return; }

    it->second->Destroy(); // 予約だけ
}

inline void GameObject::RemoveFromTypeIndex(IComponent* c)
{
	if (!c) { return; }

	// 型IDでインデックスされているコンテナから削除
	auto tid = c->GetTypeId();
	auto it = m_ComponentsByType.find(tid);
	if (it == m_ComponentsByType.end()) { return; }

	auto& v = it->second;
	v.erase(std::remove(v.begin(), v.end(), c), v.end());
	if (v.empty()) 
	{
		m_ComponentsByType.erase(it);
	}
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

// 指定型バケットからコンポーネントを削除するヘルパ
void GameObject::EraseFromBucket(IComponent::TypeId tid, IComponent* c)
{
	auto it = m_ComponentsByType.find(tid);
	if (it == m_ComponentsByType.end()) { return; }

	auto& v = it->second;
	v.erase(std::remove(v.begin(), v.end(), c), v.end());
	if (v.empty())
	{
		m_ComponentsByType.erase(it);
	}
}

void GameObject::IndexComponent(IComponent* c)
{
	if (!c) { return; }

	// 1) 実型バケット
	m_ComponentsByType[c->GetTypeId()].push_back(c);

	// 2) 基底バケット（欲しいものだけ追加）
	if (c->IsA<IRenderer>())
		m_ComponentsByType[IComponent::TypeIdOf<IRenderer>()].push_back(c);

	if (c->IsA<PhysicsComponent>())
		m_ComponentsByType[IComponent::TypeIdOf<PhysicsComponent>()].push_back(c);
}

void GameObject::UnindexComponent(IComponent* c)
{
	if (!c) { return; }

	// 実型
	EraseFromBucket(c->GetTypeId(), c);

	// 基底（IndexComponent と同じ条件で抜く）
	if (c->IsA<IRenderer>())
		EraseFromBucket(IComponent::TypeIdOf<IRenderer>(), c);

	if (c->IsA<PhysicsComponent>())
		EraseFromBucket(IComponent::TypeIdOf<PhysicsComponent>(), c);
}


// 破棄予約されたコンポーネントを破棄する
void GameObject::FlushDestroyComponents(void)
{
	// 破棄予約されたコンポーネントを破棄する
	for (auto it = m_Components.begin(); it != m_Components.end(); )
	{
		IComponent* c = it->second.get();

		if (c && c->IsDestroyRequested())
		{
			// 型索引から除外
			UnindexComponent(c);

			// 初期化キュー側にも残っている可能性があるので先に掃除
			m_InitializeQueue.erase(
				std::remove(m_InitializeQueue.begin(), m_InitializeQueue.end(), c),
				m_InitializeQueue.end()
			);

			c->Uninit();
			c->Detach();
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
	for (auto& component : m_Components)
	{
		if (component.second->GetIsValid())
		{
			component.second->Draw();
		}
	}
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
	m_ComponentsByType.clear();
}