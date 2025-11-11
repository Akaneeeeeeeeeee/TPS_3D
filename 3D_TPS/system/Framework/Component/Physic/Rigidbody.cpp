#include "RigidBody.h"
#include "Framework/EngineContext/EngineContext.h"
#include "Framework/GameObject/GameObject.h"
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include "Framework/Component/Physic/BoxCollider.h"

using namespace JPH;

Rigidbody::Rigidbody(const float mass)
    : PhysicsComponent()
    , m_Mass(mass)
{
}

void Rigidbody::Init()
{
    if (!m_Physics) { return; }
    auto& bi = m_Physics->GetBodyInterface();
    CreateBody(bi);
}

/*
* @brief	更新処理
* @param	dt	デルタタイム
* @remark	物理演算結果を元にオブジェクトの位置を更新する
* @remark	スレッドセーフな書き方になってるはず
*/
void Rigidbody::Update(float dt)
{
    if (!m_Physics || m_BodyID.IsInvalid()) { return; }

    auto& bi = m_Physics->GetBodyInterface();

    if (m_BodyType == Type::Dynamic)
    {
        RVec3 pos = bi.GetPosition(m_BodyID);
        m_pOwner->SetPosition(Vector3(pos.GetX(), pos.GetY(), pos.GetZ()));
    }
    else if (m_BodyType == Type::Kinematic)
    {
        Vector3 pos = m_pOwner->GetPosition();
        bi.SetPosition(m_BodyID, RVec3(pos.x, pos.y, pos.z), JPH::EActivation::DontActivate);
        bi.SetLinearVelocity(m_BodyID, Vec3::sZero());
        bi.SetAngularVelocity(m_BodyID, Vec3::sZero());
    }

    // Static は更新なし
}


void Rigidbody::Uninit()
{
    if (m_Physics)
    {
        DestroyBody(m_Physics->GetBodyInterface());
    }
}


void Rigidbody::Attach(EngineContext& context)
{
    PhysicsComponent::Attach(context);
}

void Rigidbody::Detach(EngineContext& context)
{
    PhysicsComponent::Detach(context);
}


// Rigidbody
void Rigidbody::CreateBody(JPH::BodyInterface& bi)
{
    auto collider = m_pOwner->GetComponent<BoxCollider>();
    if (!collider) return;

    JPH::RefConst<JPH::Shape> shape = collider->GetShape();
    if (!shape) return;

    BodyCreationSettings settings(
        shape,
        Vec3(m_pOwner->GetPosition().x, m_pOwner->GetPosition().y, m_pOwner->GetPosition().z),
        Quat::sIdentity(),
        EMotionType::Dynamic,
        Layers::MOVING
    );
    settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = m_Mass;

    Body* body = bi.CreateBody(settings);
    if (body)
    {
        m_BodyID = body->GetID();
        bi.AddBody(m_BodyID, EActivation::Activate);
        bi.ActivateBody(m_BodyID);
    }
}