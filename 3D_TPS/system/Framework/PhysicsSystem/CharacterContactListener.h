#pragma once
#include "Framework/PhysicsSystem/Physics.h"
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/Body.h>

class PhysicsManager;

class CharacterContactListenerImpl final : public JPH::CharacterContactListener
{
public:
    explicit CharacterContactListenerImpl(PhysicsManager& owner);
    ~CharacterContactListenerImpl() = default;

    // キャラクターが Body に当たったとき
    void OnContactAdded(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        const JPH::SubShapeID& inSubShapeID2,
        JPH::RVec3Arg                   inContactPosition,
        JPH::Vec3Arg                    inContactNormal,
        JPH::CharacterContactSettings& ioSettings
    ) override;
private:
    PhysicsManager& m_Owner;
};
