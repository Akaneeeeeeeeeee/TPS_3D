#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"

// Jolt
#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>

class PhysicsManager; // ëOï˚êÈåæ

class PhysicsComponent : public IComponent
{
public:
    virtual ~PhysicsComponent() noexcept override = default;

    virtual void Init(void) override = 0;
    virtual void Update(const float deltatime) override = 0;
    virtual void Uninit(void) override;

    virtual void CreateBody(JPH::BodyInterface& bi) = 0;
    virtual void DestroyBody(JPH::BodyInterface& bi);   

	virtual void Attach(EngineContext& context) override;
    virtual void Detach(EngineContext& context) override;

protected:
    PhysicsComponent() = default;
    JPH::BodyID m_BodyID;
    PhysicsManager* m_Physics = nullptr;
};
