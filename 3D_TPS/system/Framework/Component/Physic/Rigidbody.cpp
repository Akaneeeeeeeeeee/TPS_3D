#include "RigidBody.h"
#include "Framework/EngineSystem/EngineSystem.h"
#include "Framework/GameObject/GameObject.h"
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include "Framework/Component/Physic/BoxCollider.h"
#include <iostream>

using namespace JPH;

Rigidbody::Rigidbody(const float mass)
    : PhysicsComponent(),
    m_Mass(mass)
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
		// 位置を同期
        RVec3 pos = bi.GetPosition(m_BodyID);
        m_pOwner->SetPosition(Vector3(pos.GetX(), pos.GetY(), pos.GetZ()));
        // 回転も同期
        JPH::Quat rot = bi.GetRotation(m_BodyID);
        m_pOwner->SetRotation(Quaternion(rot.GetX(), rot.GetY(), rot.GetZ(), rot.GetW()));
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


void Rigidbody::Attach(EngineServices& context)
{
    PhysicsComponent::Attach(context);
}

void Rigidbody::Detach(void)
{
    PhysicsComponent::Detach();
}


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

		set.mMaxLinearVelocity = m_MaxLinearVelocity.value_or(1000.0f); // デフォルト 1000
        set.mFriction = 1.2f;        // まず 1.0～2.0 で調整
        set.mRestitution = 0.0f;     // 跳ねさせない(0だとOnCollisionExitのイベント発火で例外。0.5だとエラー出ないが、対策が必要)
        set.mLinearDamping = 0.4f;   // 横滑りを止める（0.2～1.0）
        set.mAngularDamping = 0.02f;  // 転がりを止める（0.5～2.0）
        set.mAllowSleeping = true;   // じっとしたら寝る（止まる）

        if (m_BodyType == Type::Dynamic)
        {
            set.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
            set.mMassPropertiesOverride.mMass = m_Mass; // ← 質量
        }

        // トリガーならセンサーに
        if (m_IsTrigger)
        {
            set.mIsSensor = true;
        }

        // GameObject* を UserData に入れる
        set.mUserData = reinterpret_cast<JPH::uint64>(m_pOwner);
        
        if (Body* body = bi.CreateBody(set))
        {
            m_BodyID = body->GetID();
            bi.AddBody(m_BodyID, EActivation::Activate);

            // 初回生成でも保留速度を反映
            if (m_PendingLinearVelocity.has_value())
            {
                const Vector3 v = *m_PendingLinearVelocity;
                bi.SetLinearVelocity(m_BodyID, JPH::Vec3(v.x, v.y, v.z));
                bi.ActivateBody(m_BodyID);
                m_PendingLinearVelocity.reset();
            }

            // 角速度pending
            if (m_PendingAngularVelocity.has_value())
            {
                const Vector3 w = *m_PendingAngularVelocity;
                bi.SetAngularVelocity(m_BodyID, JPH::Vec3(w.x, w.y, w.z));
                bi.ActivateBody(m_BodyID);
                m_PendingAngularVelocity.reset();
            }
        }
    }
    else
    {
        // 既存ボディ：形状差し替え
        bi.SetShape(m_BodyID, final_shape, true, EActivation::Activate);
		bi.SetMaxLinearVelocity(m_BodyID, m_MaxLinearVelocity.value_or(1000.0f)); // デフォルト 1000

        // 生成直後に保留していた初速を適用
        if (m_PendingLinearVelocity.has_value())
        {
            const Vector3 v = *m_PendingLinearVelocity;
            auto& bi2 = m_Physics->GetBodyInterface();
            bi2.SetLinearVelocity(m_BodyID, JPH::Vec3(v.x, v.y, v.z));
            bi2.ActivateBody(m_BodyID);
            m_PendingLinearVelocity.reset();
        }

        // 角速度pending
        if (m_PendingAngularVelocity.has_value())
        {
            const Vector3 w = *m_PendingAngularVelocity;
            bi.SetAngularVelocity(m_BodyID, JPH::Vec3(w.x, w.y, w.z));
            bi.ActivateBody(m_BodyID);
            m_PendingAngularVelocity.reset();
        }

        // Dynamic のとき、質量を指定値に合わせ直す
        if (m_BodyType == Type::Dynamic)
        {
            auto mp = final_shape->GetMassProperties();
            mp.ScaleToMass(m_Mass);

            BodyLockWrite lock(m_Physics->GetSystem().GetBodyLockInterface(), m_BodyID);
            if (lock.Succeeded())
            {
                lock.GetBody().GetMotionProperties()->SetMassProperties(EAllowedDOFs::All, mp);
            }
        }

        bi.ActivateBody(m_BodyID);
    }

    // 4) 参照保持
    mCompoundShape = final_shape;
}

void Rigidbody::SetLinearVelocity(const Vector3& v, bool activate)
{
    if (!m_Physics || m_BodyID.IsInvalid()) { return; }

    auto& bi = m_Physics->GetBodyInterface();
    bi.SetLinearVelocity(m_BodyID, JPH::Vec3(v.x, v.y, v.z));
    JPH::Vec3 w = bi.GetAngularVelocity(m_BodyID);
    std::cout << "[RB] angVel=(" << w.GetX() << "," << w.GetY() << "," << w.GetZ() << ")\n";

    if (activate)
    {
        bi.ActivateBody(m_BodyID);
    }
}

void Rigidbody::SetInitialVelocity(const Vector3& v)
{
    if (!m_Physics || m_BodyID.IsInvalid())
    {
        m_PendingLinearVelocity = v;
        return;
    }
    SetLinearVelocity(v, true);
}

Vector3 Rigidbody::GetLinearVelocity(void)
{
    if (m_Physics && !m_BodyID.IsInvalid())
    {
        // PhysicsSystem を取得
        auto& system = m_Physics->GetSystem();
        // BodyLockInterfaceNoLock はコピー不可なので、参照で取得する
        auto& lock_interface = system.GetBodyLockInterfaceNoLock();
        // 読み取りロック
        JPH::BodyLockRead lock(lock_interface, m_BodyID);
        if (lock.Succeeded())
        {
            const JPH::Body& body = lock.GetBody();
            JPH::Vec3 vel = body.GetLinearVelocity();
            return Vector3(vel.GetX(), vel.GetY(), vel.GetZ());
        }
    }
    return Vector3(0.0f, 0.0f, 0.0f);
}

void Rigidbody::SetMaxLinearVelocity(float maxVel, bool applyNow)
{
    // 変な値ガード（0以下は事故りやすい）
    if (maxVel <= 0.0f) return;

    m_MaxLinearVelocity = maxVel;

    if (!applyNow) return;
    if (!m_Physics || m_BodyID.IsInvalid()) return;

    auto& bi = m_Physics->GetBodyInterface();
    bi.SetMaxLinearVelocity(m_BodyID, m_MaxLinearVelocity.value());
}

void Rigidbody::SetAngularVelocity(const Vector3& w, bool activate)
{
    if (!m_Physics || m_BodyID.IsInvalid()) { return; }

    auto& bi = m_Physics->GetBodyInterface();
    bi.SetAngularVelocity(m_BodyID, JPH::Vec3(w.x, w.y, w.z));
    if (activate) bi.ActivateBody(m_BodyID);
}

void Rigidbody::SetInitialAngularVelocity(const Vector3& w)
{
    if (!m_Physics || m_BodyID.IsInvalid())
    {
        m_PendingAngularVelocity = w; // Bodyが無ければ保留
        return;
    }
    SetAngularVelocity(w, true);
}
