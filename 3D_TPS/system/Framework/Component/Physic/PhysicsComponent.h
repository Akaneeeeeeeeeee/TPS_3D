#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include "Framework/PhysicsSystem/Physics.h"    // Jolt
#include "renderer.h"

// ëOï˚êÈåæ
class PhysicsManager; 

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

    virtual void SetMesh(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices) {};

protected:
    PhysicsComponent() = default;
    JPH::BodyID m_BodyID;
    PhysicsManager* m_Physics = nullptr;
};
