#pragma once
#include "PhysicsComponent.h"

//#include <Jolt/Physics/Body/BodyInterface.h>
//#include <Jolt/Physics/Body/BodyCreationSettings.h>
//#include <Jolt/Physics/Collision/Shape/BoxShape.h>
//#include <Jolt/Math/Vec3.h>
#include "Framework/PhysicsSystem/Physics.h"
#include "commontypes.h"

class PhysicsManager;

class BoxCollider : public PhysicsComponent
{
public:
    BoxCollider(const DirectX::XMFLOAT3& halfExtent);
    ~BoxCollider() noexcept override = default;

    void CreateBody(JPH::BodyInterface& bi) override;

    void Init() override;
    void Update(const float deltaTime) override;
    void Uninit() override;

protected:
    void Attach(EngineContext& context) override;
    void Detach(EngineContext& context) override;

private:
    DirectX::XMFLOAT3 m_HalfExtent;

    JPH::Ref<JPH::BoxShape> m_Shape = nullptr;
};
