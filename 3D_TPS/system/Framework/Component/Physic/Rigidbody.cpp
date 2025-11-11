#include "RigidBody.h"
#include "Framework/EngineContext/EngineContext.h"
#include "Framework/GameObject/GameObject.h"
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

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
    context.joltPhysicsManager.Register(this);
}

void Rigidbody::Detach(EngineContext& context)
{
    context.joltPhysicsManager.UnRegister(this);
}


void Rigidbody::CreateBody(JPH::BodyInterface& bi)
{
    /*using namespace JPH;

    Vector3 scale = m_pOwner->GetScale();
    RefConst<Shape> shape = new BoxShape(Vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f));

    EMotionType motion = EMotionType::Dynamic;
    switch (m_BodyType)
    {
    case Type::Static: motion = EMotionType::Static; break;
    case Type::Kinematic: motion = EMotionType::Kinematic; break;
    case Type::Dynamic: motion = EMotionType::Dynamic; break;
    }

    BodyCreationSettings settings(
        shape,
        Vec3(m_pOwner->GetPosition().x, m_pOwner->GetPosition().y, m_pOwner->GetPosition().z),
        Quat::sIdentity(),
        motion,
        Layers::MOVING
    );

    settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    settings.mMassPropertiesOverride.mMass = m_Mass;

    Body* body = bi.CreateBody(settings);
    m_BodyID = body->GetID();
    bi.AddBody(m_BodyID, EActivation::Activate);*/
}