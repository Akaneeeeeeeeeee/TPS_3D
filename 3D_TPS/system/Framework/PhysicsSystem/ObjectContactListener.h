#pragma once
#include "Framework/PhysicsSystem/Physics.h"
#include "system/commontypes.h"

class PhysicsComponent;

enum class ContactPhase
{
    Enter,
    Stay,
    Exit,
};

struct CollisionInfo
{
    PhysicsComponent* self = nullptr;   // 自身
    PhysicsComponent* other = nullptr;  // 衝突相手

    JPH::ObjectLayer selfLayer = 0;
    JPH::ObjectLayer otherLayer = 0;

    Vector3 position; // 接触点の世界座標
    Vector3 normal;   // 接触面の法線
};

struct PhysicsContactEvent
{
    ContactPhase phase;
    CollisionInfo info;
};



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
