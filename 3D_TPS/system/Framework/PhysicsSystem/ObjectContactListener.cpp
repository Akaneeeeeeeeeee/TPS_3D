#include "ObjectContactListener.h"
#include "Framework/PhysicsSystem/PhysicsLayer.h"
#include <Jolt/Physics/Collision/ContactListener.h>
#include "Framework/GameObject/GameObject.h"

ObjectContactListener::ObjectContactListener(PhysicsManager& owner)
    : m_Owner(owner)
{
}


ValidateResult ObjectContactListener::OnContactValidate(
    const JPH::Body& body1,
    const JPH::Body& body2,
    JPH::RVec3Arg baseOffset,
    const JPH::CollideShapeResult& result)
{
    // Jolt 側（userData → GameObject*）はポインタ
    // null の可能性があるのでポインタが向いている
    // エンジン内の「接触イベント API」は参照にする
    GameObject* go1 = FromUserData(body1.GetUserData());
    GameObject* go2 = FromUserData(body2.GetUserData());
    if (!go1 || !go2) { return ValidateResult::RejectAllContactsForThisBodyPair; }
    // ここでフィルタリングを行うこともできる
    return ValidateResult::AcceptAllContactsForThisBodyPair;
}

void ObjectContactListener::OnContactAdded(
    const JPH::Body& body1,
    const JPH::Body& body2,
    const JPH::ContactManifold& manifold,
    JPH::ContactSettings& settings)
{
    // Jolt 側（userData → GameObject*）はポインタ
    // null の可能性があるのでポインタが向いている
    // エンジン内の「接触イベント API」は参照にする
    GameObject* go1 = FromUserData(body1.GetUserData());
    GameObject* go2 = FromUserData(body2.GetUserData());

    if (!go1 || !go2) { return; }

    const bool sensor1 = body1.IsSensor();
    const bool sensor2 = body2.IsSensor();

    if (sensor1 && !sensor2)
    {
        // trigger1 に物体2が入った
        m_Owner.OnTriggerEnter(*go1, *go2);
    }
    else if (!sensor1 && sensor2)
    {
        // trigger2 に物体1が入った
        m_Owner.OnTriggerEnter(*go2, *go1);
    }
    else
    {
        // 通常の衝突
        m_Owner.OnCollisionEnter(*go1, *go2);
    }
}


void ObjectContactListener::OnContactPersisted(
    const JPH::Body& body1,
    const JPH::Body& body2,
    const JPH::ContactManifold& manifold,
    JPH::ContactSettings& settings)
{
    GameObject* go1 = FromUserData(body1.GetUserData());
    GameObject* go2 = FromUserData(body2.GetUserData());
    
    if (!go1 || !go2) { return; }

    const bool sensor1 = body1.IsSensor();
    const bool sensor2 = body2.IsSensor();

    if (sensor1 && !sensor2)
    {
        m_Owner.OnTriggerStay(*go1, *go2);
    }
    else if (!sensor1 && sensor2)
    {
        m_Owner.OnTriggerStay(*go2, *go1);
    }
    else
    {
        m_Owner.OnCollisionStay(*go1, *go2);
    }
}

void ObjectContactListener::OnContactRemoved(const JPH::SubShapeIDPair& pair)
{
    // BodyID から Body を取得
    auto& system = m_Owner.GetSystem();
    const JPH::BodyInterface& bodyInterface = system.GetBodyInterfaceNoLock();

    // BodyLockRead を使って Body を取得
    JPH::BodyLockRead lock1(system.GetBodyLockInterfaceNoLock(), pair.GetBody1ID());
    JPH::BodyLockRead lock2(system.GetBodyLockInterfaceNoLock(), pair.GetBody2ID());

    const JPH::Body* body1 = &lock1.GetBody();
    const JPH::Body* body2 = &lock2.GetBody();

    if (!body1 || !body2) { return; }

    GameObject* go1 = FromUserData(body1->GetUserData());
    GameObject* go2 = FromUserData(body2->GetUserData());

    if (!go1 || !go2) { return; }

    const bool sensor1 = body1->IsSensor();
    const bool sensor2 = body2->IsSensor();

    if (sensor1 && !sensor2)
    {
        m_Owner.OnTriggerExit(*go1, *go2);
    }
    else if (!sensor1 && sensor2)
    {
        m_Owner.OnTriggerExit(*go2, *go1);
    }
    else
    {
        m_Owner.OnCollisionExit(*go1, *go2);
    }
}
