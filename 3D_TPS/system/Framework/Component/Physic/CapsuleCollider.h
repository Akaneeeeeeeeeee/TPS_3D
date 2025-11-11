#pragma once
#include "PhysicsComponent.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

class CapsuleCollider : public PhysicsComponent
{
public:
    CapsuleCollider()
        : m_HalfHeight(0.0f), m_Radius(0.0f) {
    }

    void Init(void) override;
    void Update(const float deltaTime) override {};
    void Uninit(void) override;

	void SetHalfHeight(const float halfHeight) { m_HalfHeight = halfHeight; }
	void SetRadius(const float radius) { m_Radius = radius; }
    void SetSize(const float halfHeight, const float radius) {
        m_HalfHeight = halfHeight;
        m_Radius = radius;
	}

protected:
    void Attach(EngineContext& context) override;
    void Detach(EngineContext& context) override;

	void CreateBody(JPH::BodyInterface& bi) override;

private:
    float m_HalfHeight;
    float m_Radius;
    JPH::RefConst<JPH::CapsuleShape> m_Shape = nullptr;
};
