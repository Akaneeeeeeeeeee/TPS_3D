#include "CharacterContactListener.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"

CharacterContactListenerImpl::CharacterContactListenerImpl(PhysicsManager& owner)
    : m_Owner(owner)
{
}

bool CharacterContactListenerImpl::OnContactValidate(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    const JPH::SubShapeID& inSubShapeID2)
{
    // ここでfalseにすると「キャラ側の当たり判定」そのものが無効になるので基本true
    return true;
}

bool CharacterContactListenerImpl::ReadUserDataAndFlags(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    uint64_t& outCharId,
    uint64_t& outOtherId,
    bool& outIsTrigger,
    JPH::BodyID& outInnerBodyID)
{
    outCharId = 0;
    outOtherId = 0;
    outIsTrigger = false;

    outInnerBodyID = inCharacter->GetInnerBodyID();

    auto& system = m_Owner.GetSystem();
    auto& lockIF = system.GetBodyLockInterface();

    // 1) Character側（InnerBody）からID取得
    {
        JPH::BodyLockRead lock(lockIF, outInnerBodyID);
        if (!lock.Succeeded()) return false;
        const JPH::Body& innerBody = lock.GetBody();
        outCharId = static_cast<uint64_t>(innerBody.GetUserData());
        // Character側をセンサーにしているならここも見る（通常はfalse）
        // bool charIsSensor = innerBody.IsSensor();
    }

    // 2) 相手BodyからID/センサー判定取得（Removedではやらない）
    {
        JPH::BodyLockRead lock(lockIF, inBodyID2);
        if (!lock.Succeeded()) return false;
        const JPH::Body& body2 = lock.GetBody();
        outOtherId = static_cast<uint64_t>(body2.GetUserData());

        const bool otherIsSensor = body2.IsSensor();
        outIsTrigger = otherIsSensor;
    }

    if (outCharId == 0 || outOtherId == 0) return false;
    return true;
}

#if (JPH_VERSION_MAJOR > 5) || (JPH_VERSION_MAJOR == 5 && JPH_VERSION_MINOR >= 3)

void CharacterContactListenerImpl::OnContactAdded(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    const JPH::SubShapeID& inSubShapeID2,
    JPH::RVec3Arg inContactPosition,
    JPH::Vec3Arg  inContactNormal,
    JPH::CharacterContactSettings& ioSettings)
{
    uint64_t charId = 0, otherId = 0;
    bool isTrigger = false;
    JPH::BodyID innerBodyID;

    if (!ReadUserDataAndFlags(inCharacter, inBodyID2, charId, otherId, isTrigger, innerBodyID))
        return;

    const auto key = MakeKey(innerBodyID, inBodyID2);

    bool needEnter = false;
    {
        std::lock_guard<std::mutex> lk(m_PairMtx);
        auto& info = m_Pairs[key];

        if (info.refCount == 0)
        {
            info.isTrigger = isTrigger;
            info.charId = charId;
            info.otherId = otherId;
            needEnter = true;
        }
        info.refCount++;
    }

    if (!needEnter) return;

    // センサー相手なら TriggerEnter、そうでなければ CharacterEnter
    if (isTrigger)
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::TriggerEnter, charId, otherId);
    else
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::CharacterEnter, charId, otherId);
}

void CharacterContactListenerImpl::OnContactPersisted(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    const JPH::SubShapeID& inSubShapeID2,
    JPH::RVec3Arg inContactPosition,
    JPH::Vec3Arg  inContactNormal,
    JPH::CharacterContactSettings& ioSettings)
{
    // 今回は Stay を出さない（必要になったらここで積む）
}

void CharacterContactListenerImpl::OnContactRemoved(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    const JPH::SubShapeID& inSubShapeID2)
{
    // ★重要：RemovedではBodyが消えている可能性があるので、BodyをLockして触らない :contentReference[oaicite:3]{index=3}
    const JPH::BodyID innerBodyID = inCharacter->GetInnerBodyID();
    const auto key = MakeKey(innerBodyID, inBodyID2);

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
            info = it->second; // Exit用にコピー
            m_Pairs.erase(it);
            needExit = true;
        }
    }

    if (!needExit) return;
    if (info.charId == 0 || info.otherId == 0) return;

    if (info.isTrigger)
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::TriggerExit, info.charId, info.otherId);
    else
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::CharacterExit, info.charId, info.otherId);
}

#else

void CharacterContactListenerImpl::OnContactAdded(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    const JPH::SubShapeID& inSubShapeID2,
    JPH::RVec3Arg inContactPosition,
    JPH::Vec3Arg  inContactNormal,
    JPH::CharacterContactSettings& ioSettings)
{
    // v5.1系：Removedが無いので「Enter重複抑制」だけやる（Exitは別方式が必要）
    uint64_t charId = 0, otherId = 0;
    bool isTrigger = false;
    JPH::BodyID innerBodyID;

    if (!ReadUserDataAndFlags(inCharacter, inBodyID2, charId, otherId, isTrigger, innerBodyID))
        return;

    const auto key = MakeKey(innerBodyID, inBodyID2);

    bool needEnter = false;
    {
        std::lock_guard<std::mutex> lk(m_PairMtx);
        auto& info = m_Pairs[key];
        if (info.refCount == 0)
        {
            info.isTrigger = isTrigger;
            info.charId = charId;
            info.otherId = otherId;
            needEnter = true;
        }
        // v5.1のOnContactAddedは毎回呼ばれがちなので、refCountは増やしっぱなしにしない
        // （ここでは「存在フラグ」扱いにして 1 固定にする）
        info.refCount = 1;
    }

    if (!needEnter) return;

    if (isTrigger)
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::TriggerEnter, charId, otherId);
    else
        m_Owner.EnqueueEvent(PhysicsManager::CollisionEvent::Type::CharacterEnter, charId, otherId);
}

#endif
