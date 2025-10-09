#include "IComponent.h"
#include "system/Framework/GameObject/GameObject.h"

IComponent::IComponent(GameObject* owner) :m_pOwner(owner)
{
}

IComponent::~IComponent()
{
	m_pOwner = nullptr;
}

// そのコンポーネントを持つオブジェクトの割り当て(引数：オブジェクト(参照))
void IComponent::SetOwner(GameObject* _obj) {
	// 参照したオブジェクトのアドレスを&で代入
	m_pOwner = _obj;
}

// アタッチ先のオブジェクトの取得
GameObject* IComponent::GetOwner(void) {
	return m_pOwner;
}


