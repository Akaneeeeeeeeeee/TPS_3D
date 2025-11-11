#include "BoxCollider.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/GameObject/GameObject.h"
#include "Framework/PhysicsSystem/PhysicsLayer.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include <Jolt/Physics/EActivation.h>

BoxCollider::BoxCollider(void)
	: m_HalfSize(Vector3::Zero), PhysicsComponent()
{
}

void BoxCollider::Attach(EngineContext& context)
{
    PhysicsComponent::Attach(context);
    Vector3 scale = m_pOwner->GetScale();
    m_Shape = new JPH::BoxShape(JPH::Vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f));
}

void BoxCollider::Init()
{
    if (!m_Physics) { return; }
    auto& bi = m_Physics->GetBodyInterface();
    CreateBody(bi);
}

void BoxCollider::Update(const float deltaTime)
{
    if (!m_Physics) return;

    // Rigidbody が付いている場合は物理が制御するので同期しない
    if (m_pOwner->GetComponent<Rigidbody>()) { return; }

    auto& bi = m_Physics->GetBodyInterface();
    if (bi.IsAdded(m_BodyID))
    {
        Vector3 pos = m_pOwner->GetPosition();
        bi.SetPosition(m_BodyID, JPH::RVec3(pos.x, pos.y, pos.z), JPH::EActivation::DontActivate);
    }
}

void BoxCollider::Uninit()
{
    // JPH::BodyID には IsValid() メソッドが無いので、IDが無効かどうかは BodyInterface::IsActive などで判定する
    if (m_Physics && m_Physics->GetBodyInterface().IsAdded(m_BodyID))
    {
        m_Physics->GetBodyInterface().RemoveBody(m_BodyID);
    }
}

void BoxCollider::Detach(EngineContext& context)
{
	m_Shape = nullptr;
    PhysicsComponent::Detach(context);
}


void BoxCollider::CreateBody(JPH::BodyInterface& bi)
{
    JPH::Quat q(m_pOwner->GetRotation().x, m_pOwner->GetRotation().y,
        m_pOwner->GetRotation().z, m_pOwner->GetRotation().w);

    JPH::BodyCreationSettings settings(
        m_Shape,
        JPH::RVec3(m_pOwner->GetPosition().x, m_pOwner->GetPosition().y, m_pOwner->GetPosition().z),
        q,
        JPH::EMotionType::Kinematic,
        Layers::NON_MOVING
    );

    JPH::Body* body = bi.CreateBody(settings);
    if (!body)
    {
        OutputDebugStringA("Failed to create body!\n");
        return;
    }
    m_BodyID = body->GetID();
    bi.AddBody(m_BodyID, JPH::EActivation::Activate);
}
