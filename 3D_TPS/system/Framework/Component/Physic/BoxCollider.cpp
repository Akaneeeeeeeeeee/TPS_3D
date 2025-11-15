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
}

void BoxCollider::Init()
{
    if (!m_Physics) { return; }

    Vector3 scale = m_pOwner->GetScale();
    m_Shape = new JPH::BoxShape(JPH::Vec3(scale.x, scale.y, scale.z));
    //m_Shape = new JPH::BoxShape(JPH::Vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f));

    // Rigidbody が付いているなら Body は作らない（Rigidbody がまとめて作る）
    if (m_pOwner->GetComponent<Rigidbody>() == nullptr) {
        auto& bi = m_Physics->GetBodyInterface();
        //CreateBody(bi);
    }
    else {
        // 念のため無効 ID にして、Update で触らないようにしておく
        m_BodyID = JPH::BodyID();
    }
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
        m_Shape,                                // その Body が持つ「衝突形状」(AABB/慣性計算のベース)
        JPH::RVec3(                             // Body の初期ワールド位置（重心位置）
            m_pOwner->GetPosition().x, 
            m_pOwner->GetPosition().y, 
            m_pOwner->GetPosition().z),
        q,                                      // Body の初期ワールド回転
        JPH::EMotionType::Kinematic,            // Static / Kinematic / Dynamic
        Layers::NON_MOVING                      // 衝突レイヤ（フィルタリング用）
    );

    // 生成チェック
    if (JPH::Body* body = bi.CreateBody(settings)) {
        m_BodyID = body->GetID();
        bi.AddBody(m_BodyID, JPH::EActivation::Activate);
    }
    else {
        OutputDebugStringA("Failed to create body!\n");
    }
}
