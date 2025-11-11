#include "CapsuleCollider.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/GameObject/GameObject.h"

void CapsuleCollider::Init(void)
{
    if (!m_Physics) return;
    CreateBody(m_Physics->GetBodyInterface());
}

void CapsuleCollider::Uninit(void)
{
    if (m_Physics && m_Physics->GetBodyInterface().IsAdded(m_BodyID))
    {
        m_Physics->GetBodyInterface().RemoveBody(m_BodyID);
    }
}

void CapsuleCollider::Attach(EngineContext& context)
{
    PhysicsComponent::Attach(context);
    m_Shape = new JPH::CapsuleShape(m_HalfHeight, m_Radius);
}

void CapsuleCollider::Detach(EngineContext& context)
{
    m_Shape = nullptr;
    PhysicsComponent::Detach(context);
}

void CapsuleCollider::CreateBody(JPH::BodyInterface& bi)
{
    JPH::Quat q(m_pOwner->GetRotation().x, m_pOwner->GetRotation().y,
        m_pOwner->GetRotation().z, m_pOwner->GetRotation().w);

    JPH::BodyCreationSettings settings(
        m_Shape,
        JPH::RVec3(m_pOwner->GetPosition().x, m_pOwner->GetPosition().y, m_pOwner->GetPosition().z),
        q,
        JPH::EMotionType::Dynamic,
        Layers::MOVING
    );

    settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = 1.0f;

    JPH::Body* body = bi.CreateBody(settings);
    m_BodyID = body->GetID();
    bi.AddBody(m_BodyID, JPH::EActivation::Activate);
}