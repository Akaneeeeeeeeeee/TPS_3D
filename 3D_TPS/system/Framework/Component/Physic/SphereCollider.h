#pragma once
#include "Framework/Component/Physic/PhysicsComponent.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

class SphereCollider final : public PhysicsComponent
{
public:
    SphereCollider() = default;
    ~SphereCollider() override = default;

    void Init(void) override;
    void Update(const float) override {}
    void Uninit(void) override;

    void Attach(EngineServices& context) override;
    void Detach(void) override;

    bool IsCollider(void) const noexcept override { return true; }
    JPH::RefConst<JPH::Shape> GetShape(void) const override { return JPH::RefConst<JPH::Shape>(m_Shape); }

    void SetRadius(float r) { m_Radius = r; }
    float GetRadius() const { return m_Radius; }

private:
    float m_Radius = 0.0f;
    JPH::RefConst<JPH::SphereShape> m_Shape = nullptr;
};
