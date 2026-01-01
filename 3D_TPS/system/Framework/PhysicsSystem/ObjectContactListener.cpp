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
    // ここで Reject すると「衝突そのもの」が無効になるので基本Acceptにする
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
    // ※現在は userData に GameObject の ID(uint64) を入れている

    const uint64_t id1 = static_cast<uint64_t>(body1.GetUserData());
    const uint64_t id2 = static_cast<uint64_t>(body2.GetUserData());
    if (id1 == 0 || id2 == 0) return; // 物理は有効のまま、イベントだけ出さない

    const bool sensor1 = body1.IsSensor();
    const bool sensor2 = body2.IsSensor();
    const bool isTrigger = (sensor1 && !sensor2) || (!sensor1 && sensor2);

    const auto key = MakeKey(body1.GetID(), body2.GetID());

    bool needEnter = false;
    {
        std::lock_guard<std::mutex> lk(m_PairMtx);
        auto& info = m_Pairs[key];

        if (info.refCount == 0)
        {
            info.isTrigger = isTrigger;
            info.id1 = id1;
            info.id2 = id2;
            needEnter = true;
        }
        info.refCount++;
    }

    if (needEnter)
    {
        if (isTrigger)
            m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::TriggerEnter, id1, id2);
        else
            m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::CollisionEnter, id1, id2);
    }
}

void ObjectContactListener::OnContactPersisted(
    const JPH::Body& body1,
    const JPH::Body& body2,
    const JPH::ContactManifold& manifold,
    JPH::ContactSettings& settings)
{
    // 既存の Stay を使いたい場合、
    // ここで毎回キューに積むと量が増えやすいので、まずは Enter/Exit のみで安定化させる。
    // 必要なら「メインスレッド側で接触ペア集合から毎フレームStayを呼ぶ」方式にするのが安全。
}

void ObjectContactListener::OnContactRemoved(const JPH::SubShapeIDPair& pair)
{
    // BodyID から Body を取得
    // （Removed は Body が既に消えている可能性があるので、ここで Body を取りに行かない）
    // ※Addedで保存した情報から Exit を作る

    const auto key = MakeKey(pair.GetBody1ID(), pair.GetBody2ID());

    PairInfo info{};
    bool needExit = false;

    {
        std::lock_guard<std::mutex> lk(m_PairMtx);
        auto it = m_Pairs.find(key);
        if (it == m_Pairs.end())
            return;

        it->second.refCount--;
        if (it->second.refCount <= 0)
        {
            info = it->second;      // Exit用にコピー
            m_Pairs.erase(it);
            needExit = true;
        }
    }

    if (!needExit) return;
    if (info.id1 == 0 || info.id2 == 0) return;

    if (info.isTrigger)
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::TriggerExit, info.id1, info.id2);
    else
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::CollisionExit, info.id1, info.id2);
}