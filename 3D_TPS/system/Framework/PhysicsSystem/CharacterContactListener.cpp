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
    GameObject* charGO = nullptr;
    {
        // CharacterVirtual 自体には userData を持たせていないので、
        // InnerBodyID → Body → userData から取得する
        JPH::BodyID innerID = inCharacter->GetInnerBodyID();

        auto& system = m_Owner.GetSystem();
        JPH::BodyLockRead lock(system.GetBodyLockInterfaceNoLock(), innerID);
        if (lock.Succeeded())
        {
            const JPH::Body& innerBody = lock.GetBody();
            charGO = FromUserData(innerBody.GetUserData());
        }
    }

    // 2) ぶつかった相手 Body の GameObject*
    GameObject* otherGO = nullptr;
    {
        auto& system = m_Owner.GetSystem();
        JPH::BodyLockRead lock(system.GetBodyLockInterfaceNoLock(), inBodyID2);
        if (lock.Succeeded())
        {
            const JPH::Body& body2 = lock.GetBody();
            otherGO = FromUserData(body2.GetUserData());
        }
    }

    if (!charGO || !otherGO) return;

    // ここで Tags や Body の sensor フラグを見て、衝突/トリガーを振り分けてもよい
    // まずは「キャラが何かに当たった」というイベントにまとめる
    m_Owner.OnCharacterCollisionEnter(*charGO, *otherGO);
}