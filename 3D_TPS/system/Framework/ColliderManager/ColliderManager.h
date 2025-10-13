#pragma once
#include "system/Framework/Component/Collider/3D/ICollider.h"
#include <vector>

class ColliderManager
{
public:
	/*ColliderManager();
	~ColliderManager();*/

	static void Register(ICollider* collider);
	static void UnRegister(ICollider* collider);

private:
	static std::vector<ICollider*> m_pColliders;
};

//ColliderManager::ColliderManager()
//{
//}
//
//ColliderManager::~ColliderManager()
//{
//}