#include "RigidBody.h"
#include "Framework/EngineContext/EngineContext.h"
#include "Framework/GameObject/GameObject.h"
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
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

void Rigidbody::Detach(void)
{
    PhysicsComponent::Detach();
}


// Rigidbody
//void Rigidbody::CreateBody(JPH::BodyInterface& bi)
//{
//    using namespace JPH;
//
//    // 1) コライダー収集（自分とコライダー以外は無視）
//    std::vector<PhysicsComponent*> pcs;
//    m_pOwner->GetComponents(pcs);
//
//    struct Part { RefConst<Shape> shape; RMat44 pose; };
//    std::vector<Part> parts;
//    parts.reserve(pcs.size());
//
//    for (auto* pc : pcs)
//    {
//        if (pc == this) continue;
//        if (!pc->IsCollider()) continue;
//
//        if (auto s = pc->GetShape())
//        {
//            parts.push_back(Part{ s, pc->GetLocalPose() }); // local pose は Owner ローカル基準
//        }
//    }
//
//    if (parts.empty()) { OutputDebugStringA("No colliders.\n"); return; }
//
//    // 2) 最終 Shape を決定（単品 or コンパウンド）
//    RefConst<Shape> final_shape;
//    if (parts.size() == 1)
//    {
//        final_shape = parts[0].shape;
//    }
//    else
//    {
//        // パフォ重視：StaticCompound（子の追加削除は再構築で対応）
//        StaticCompoundShapeSettings comp;
//        for (auto& p : parts)
//            comp.AddShape(p.pose.GetTranslation(), p.pose.GetQuaternion(), p.shape);
//
//        final_shape = comp.Create().Get();
//    }
//
//    // 3) Body を新規作成 or 形状差し替え
//    if (m_BodyID.IsInvalid())
//    {
//        BodyCreationSettings set(
//            final_shape,
//            RVec3(m_pOwner->GetPosition().x, m_pOwner->GetPosition().y, m_pOwner->GetPosition().z),
//            Quat(m_pOwner->GetRotation().x, m_pOwner->GetRotation().y,
//                m_pOwner->GetRotation().z, m_pOwner->GetRotation().w),
//            ToJPHMotionType(m_BodyType),    // Static / Kinematic / Dynamic 変換関数
//            Layers::MOVING                  // レイヤーは適宜
//        );
//
//        if (m_BodyType == Type::Dynamic)
//        {
//            set.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
//            set.mMassPropertiesOverride.mMass = m_Mass; // ← 質量
//        }
//
//        if (Body* body = bi.CreateBody(set))
//        {
//            m_BodyID = body->GetID();
//            bi.AddBody(m_BodyID, EActivation::Activate);
//        }
//    }
//    else
//    {
//        // 既存ボディ：形状を差し替えて活性化
//        bi.SetShape(m_BodyID, final_shape, /*inUpdateMassProperties=*/true, EActivation::Activate);
//
//        // Dynamic のとき、質量を指定値に合わせ直す（任意だが実務では安定）
//        if (m_BodyType == Type::Dynamic)
//        {
//            auto mp = final_shape->GetMassProperties();
//            mp.ScaleToMass(m_Mass);
//
//            JPH::BodyLockWrite lock(m_Physics->GetSystem().GetBodyLockInterface(), m_BodyID);
//            if (lock.Succeeded())
//            {
//                lock.GetBody().GetMotionProperties()
//                    ->SetMassProperties(JPH::EAllowedDOFs::All, mp);
//            }
//        }
//
//        bi.ActivateBody(m_BodyID);
//    }
//
//    // 4) 参照保持（寿命管理）
//    mCompoundShape = final_shape;
//}


void Rigidbody::CreateBody(JPH::BodyInterface& bi)
{
    using namespace JPH;

    // 1) コライダー収集
    std::vector<PhysicsComponent*> pcs;
    m_pOwner->GetComponents(pcs);

    struct Part
    {
        RefConst<Shape> shape;
        RMat44          pose;   // Owner ローカルのオフセット＋回転
    };
    std::vector<Part> parts;
    parts.reserve(pcs.size());

    for (auto* pc : pcs)
    {
        if (pc == this)      continue;
        if (!pc->IsCollider()) continue;

        if (auto s = pc->GetShape())
        {
            parts.push_back(Part{ s, pc->GetLocalPose() });
        }
    }

    if (parts.empty())
    {
        OutputDebugStringA("No colliders.\n");
        return;
    }

    // 2) Shape を決定（常に Compound 経由にする）
    RefConst<Shape> final_shape;

    if (parts.size() == 1)
    {
        // 単体でも Compound にする（オフセット対応のため）
        StaticCompoundShapeSettings comp;

        const Part& p = parts[0];
        Vec3  local_pos = Vec3::sZero();
        Quat  local_rot = Quat::sIdentity();

        // RMat44 から平行移動／回転を取り出す
        local_pos = p.pose.GetTranslation();
        local_rot = p.pose.GetRotation().GetQuaternion();

        comp.AddShape(local_pos, local_rot, p.shape);
        final_shape = comp.Create().Get();
    }
    else
    {
        // 複数コライダーも同様に Compound
        StaticCompoundShapeSettings comp;

        for (auto& p : parts)
        {
            Vec3 local_pos = p.pose.GetTranslation();
            Quat local_rot = p.pose.GetRotation().GetQuaternion();
            comp.AddShape(local_pos, local_rot, p.shape);
        }

        final_shape = comp.Create().Get();
    }

    // 3) Body 作成 or 差し替え（BodyCreationSettings は 5 引数版のまま）
    if (m_BodyID.IsInvalid())
    {
        BodyCreationSettings set(
            final_shape,
            RVec3(m_pOwner->GetPosition().x,
                m_pOwner->GetPosition().y,
                m_pOwner->GetPosition().z),
            Quat(m_pOwner->GetRotation().x,
                m_pOwner->GetRotation().y,
                m_pOwner->GetRotation().z,
                m_pOwner->GetRotation().w),
            ToJPHMotionType(m_BodyType),
			m_ObjectLayer				  // レイヤーは設定値を使う
        );

        if (m_BodyType == Type::Dynamic)
        {
            set.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
            set.mMassPropertiesOverride.mMass = m_Mass; // ← 質量
        }

        if (Body* body = bi.CreateBody(set))
        {
            m_BodyID = body->GetID();
            bi.AddBody(m_BodyID, EActivation::Activate);
        }
    }
    else
    {
        // 既存ボディ：形状差し替え
        bi.SetShape(m_BodyID, final_shape, true, EActivation::Activate);

        // Dynamic のとき、質量を指定値に合わせ直す（任意だが実務では安定）
        if (m_BodyType == Type::Dynamic)
        {
            auto mp = final_shape->GetMassProperties();
            mp.ScaleToMass(m_Mass);

            BodyLockWrite lock(m_Physics->GetSystem().GetBodyLockInterface(), m_BodyID);
            if (lock.Succeeded())
            {
                lock.GetBody().GetMotionProperties()
                    ->SetMassProperties(EAllowedDOFs::All, mp);
            }
        }

        bi.ActivateBody(m_BodyID);
    }

    // 4) 参照保持
    mCompoundShape = final_shape;
}
