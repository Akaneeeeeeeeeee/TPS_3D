#pragma once
#include "Framework/Component/Physic/PhysicsComponent.h"
#include "system/commontypes.h"

class Rigidbody : public PhysicsComponent
{
public:
    Rigidbody(const float mass);
    ~Rigidbody() override = default;

    enum Type {
        Static,
        Dynamic,
        Kinematic
    };

    void CreateBody(JPH::BodyInterface& bi) override;

    void Attach(EngineContext& context) override;
    void Detach(EngineContext& context) override;

    void Init(void) override;
    void Update(const float dt) override;
    void Uninit(void) override;

	void SetBodyType(const Type type) { m_BodyType = type; }

private:
	Type m_BodyType = Dynamic;
    float m_Mass = 1.0f;
    Vector3 m_Size = Vector3(1.0f, 1.0f, 1.0f);
};
