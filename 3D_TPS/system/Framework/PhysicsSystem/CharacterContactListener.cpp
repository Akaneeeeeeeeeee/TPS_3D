#include "CharacterContactListener.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/GameObject/GameObject.h"


CharacterContactListenerImpl::CharacterContactListenerImpl(PhysicsManager& owner)
    : m_Owner(owner)
{
}

void CharacterContactListenerImpl::OnContactAdded(
    const JPH::CharacterVirtual* inCharacter,
    const JPH::BodyID& inBodyID2,
    const JPH::SubShapeID& inSubShapeID2,
    JPH::RVec3Arg                   inContactPosition,
    JPH::Vec3Arg                    inContactNormal,
    JPH::CharacterContactSettings& ioSettings)
{
     // 1) Character 側 GameObject*
    uint64_t charId = 0;
    {
        // CharacterVirtual 自体には userData を持たせていないので、
        // InnerBodyID → Body → userData から取得する
        JPH::BodyID innerID = inCharacter->GetInnerBodyID();

        auto& system = m_Owner.GetSystem();
        JPH::BodyLockRead lock(system.GetBodyLockInterface(), innerID); // ★NoLockではなく通常版に（安全寄り）
        if (lock.Succeeded())
        {
            const JPH::Body& innerBody = lock.GetBody();
            charId = static_cast<uint64_t>(innerBody.GetUserData());     // ★ポインタ化しない
        }
    }

    // 2) ぶつかった相手 Body の GameObject*
    uint64_t otherId = 0;
    {
        auto& system = m_Owner.GetSystem();
        JPH::BodyLockRead lock(system.GetBodyLockInterface(), inBodyID2); // ★同上
        if (lock.Succeeded())
        {
            const JPH::Body& body2 = lock.GetBody();
            otherId = static_cast<uint64_t>(body2.GetUserData());         // ★ポインタ化しない
        }
    }

    if (charId == 0 || otherId == 0) return;

    // ここで Tags や Body の sensor フラグを見て、衝突/トリガーを振り分けてもよい
    // ここをイベントキューへ
    m_Owner.EnqueueEvent(
        PhysicsManager::CollisionEvent::Type::CharacterEnter,
        charId,
        otherId
    );
}