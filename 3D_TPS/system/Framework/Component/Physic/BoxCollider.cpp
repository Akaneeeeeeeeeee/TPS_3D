#include "BoxCollider.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/GameObject/GameObject.h"
#include "Framework/PhysicsSystem/PhysicsLayer.h"

BoxCollider::BoxCollider(const DirectX::XMFLOAT3& halfExtent)
	: m_HalfExtent(halfExtent), PhysicsComponent()
{
}

void BoxCollider::Attach(EngineContext& context)
{
    PhysicsComponent::Attach(context);
    m_Shape = new JPH::BoxShape(JPH::Vec3(m_HalfExtent.x, m_HalfExtent.y, m_HalfExtent.z));
}

void BoxCollider::Init()
{
    if (!m_Physics) { return; }
    auto& bi = m_Physics->GetBodyInterface();
    CreateBody(bi);
}

void BoxCollider::Update(const float deltaTime)
{
    // Rigidbody が無い単体 Collider の場合は Transform 同期
    if (!m_Physics) { return; }
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

    m_BodyID = bi.CreateAndAddBody(settings, JPH::EActivation::Activate);
}
