#include "PhysicsComponent.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"

void PhysicsComponent::Uninit()
{
    if (m_Physics && !m_BodyID.IsInvalid())
    {
        auto& bi = m_Physics->GetBodyInterface();
        DestroyBody(bi);
    }
}

void PhysicsComponent::DestroyBody(JPH::BodyInterface& bi)
{
    if (!m_BodyID.IsInvalid() && bi.IsAdded(m_BodyID))
    {
        bi.RemoveBody(m_BodyID);
        m_BodyID = JPH::BodyID(); // デフォルトコンストラクタで無効IDに
    }
}

void PhysicsComponent::Attach(EngineContext& context)
{
    m_Physics = &context.joltPhysicsManager;
    m_Physics->Register(this);
}

void PhysicsComponent::Detach(void)
{
    if (m_Physics)
    {
        m_Physics->UnRegister(this);
        m_Physics = nullptr;
    }
}
