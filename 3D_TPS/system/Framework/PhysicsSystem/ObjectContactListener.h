#pragma once
#include "Framework/PhysicsSystem/Physics.h"    // Jolt

class GameContactListener : public JPH::ContactListener
{
public:
    // 接触検出の前処理（必要なら）
    virtual JPH::ValidateResult OnContactValidate(
        const JPH::Body& inBody1, const JPH::Body& inBody2,
        JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult
    ) override;

    // 接触が「確定」したとき（継続もここに来る）
    virtual void OnContactAdded(
        const JPH::Body& inBody1, const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold,
        JPH::ContactSettings& ioSettings
    ) override;

    // 接触が「完全に離れた」とき
    virtual void OnContactRemoved(
        const JPH::SubShapeIDPair& inSubShapePair
    ) override;
};
