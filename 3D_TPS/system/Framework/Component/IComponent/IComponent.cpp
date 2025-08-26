#include "Src/Application/Application/Entry/main.h"
#include "IComponent.h"
#include "Src/Game/Object_3D/BaseModel/GameObject.h"

// そのコンポーネントを持つオブジェクトの割り当て(引数：オブジェクト(参照))
void IComponent::SetOwner(GameObject* _obj) {
	// 参照したオブジェクトのアドレスを&で代入
	m_pOwner = _obj;
}

// アタッチ先のオブジェクトの取得
GameObject* IComponent::GetOwner(void) {
	return m_pOwner;
}


