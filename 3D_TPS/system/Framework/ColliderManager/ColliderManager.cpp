#include "ColliderManager.h"

std::vector<ICollider*> ColliderManager::m_pColliders;

// ƒRƒ‰ƒCƒ_[“o˜^
void ColliderManager::Register(ICollider* collider)
{
	// ‘¶İ‚µ‚Ä‚¢‚È‚¯‚ê‚Î“o˜^
	if(std::find(m_pColliders.begin(), m_pColliders.end(), collider) == m_pColliders.end())
	{
		m_pColliders.push_back(collider);
	}
}

// ƒRƒ‰ƒCƒ_[“o˜^‰ğœ
void ColliderManager::UnRegister(ICollider* collider)
{
	// ‘¶İ‚µ‚Ä‚¢‚ê‚Î“o˜^‰ğœ
	auto it = std::find(m_pColliders.begin(), m_pColliders.end(), collider);
	if(it != m_pColliders.end())
	{
		m_pColliders.erase(it);
	}
}

