#include "BoxCollider.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/GameObject/GameObject.h"
#include "Framework/PhysicsSystem/PhysicsLayer.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "system/CStaticMesh.h"
#include <Jolt/Physics/EActivation.h>

BoxCollider::BoxCollider(void)
	: m_HalfSize(Vector3::Zero), PhysicsComponent()
{
}

void BoxCollider::Attach(EngineServices& context)
{
    PhysicsComponent::Attach(context);
}

void BoxCollider::Init()
{
    if (!m_Physics) { return; }

    Vector3 sc = m_pOwner ? m_pOwner->GetScale() : Vector3::One;

    // スケールを当たり判定に反映（負スケール対策でabs）
    const float hx = std::abs(m_HalfSize.x * sc.x);
    const float hy = std::abs(m_HalfSize.y * sc.y);
    const float hz = std::abs(m_HalfSize.z * sc.z);

    m_Shape = new JPH::BoxShape(JPH::Vec3(hx, hy, hz));

    // Rigidbody が付いているなら Body は作らない（Rigidbody がまとめて作る）
    if (m_pOwner->GetComponent<Rigidbody>() == nullptr) {
        auto& bi = m_Physics->GetBodyInterface();
        //CreateBody(bi);
    }
    else {
        // 念のため無効 ID にして、Update で触らないようにしておく
        m_BodyID = JPH::BodyID();
    }
}

void BoxCollider::FitToMeshLocalAABB(const CStaticMesh& mesh, float inflate)
{
    const auto& verts = mesh.GetVertices();
    if (verts.empty()) return;

    Vector3 mn = verts[0].Position;
    Vector3 mx = verts[0].Position;

    for (const auto& v : verts)
    {
        const Vector3 p = v.Position;
        mn.x = std::min(mn.x, p.x); mn.y = std::min(mn.y, p.y); mn.z = std::min(mn.z, p.z);
        mx.x = std::max(mx.x, p.x); mx.y = std::max(mx.y, p.y); mx.z = std::max(mx.z, p.z);
    }

    Vector3 half = (mx - mn) * 0.5f;
    m_HalfSize = half * inflate; // ローカル（スケール前）
}

void BoxCollider::Update(const float deltaTime)
{
    if (!m_Physics) return;

    // Rigidbody が付いている場合は物理が制御するので同期しない
    if (m_pOwner->GetComponent<Rigidbody>()) { return; }

    auto& bi = m_Physics->GetBodyInterface();
    if (bi.IsAdded(m_BodyID))
    {
        Vector3 pos = m_pOwner->GetPosition();
        bi.SetPosition(m_BodyID, JPH::RVec3(pos.x, pos.y, pos.z), JPH::EActivation::DontActivate);
    }
}

void BoxCollider::Uninit()
{
    // JPH::BodyID には IsValid() メソッドが無いので、IDが無効かどうかは BodyInterface::IsActive などで判定する
    if (m_Physics && m_Physics->GetBodyInterface().IsAdded(m_BodyID))
    {
        m_Physics->GetBodyInterface().RemoveBody(m_BodyID);
    }
}

void BoxCollider::Detach(void)
{
	m_Shape = nullptr;
    PhysicsComponent::Detach();
}


void BoxCollider::CreateBody(JPH::BodyInterface& bi)
{
    JPH::Quat q(m_pOwner->GetRotation().x, m_pOwner->GetRotation().y,
        m_pOwner->GetRotation().z, m_pOwner->GetRotation().w);

    JPH::BodyCreationSettings settings(
        m_Shape,                                // その Body が持つ「衝突形状」(AABB/慣性計算のベース)
        JPH::RVec3(                             // Body の初期ワールド位置（重心位置）
            m_pOwner->GetPosition().x, 
            m_pOwner->GetPosition().y, 
            m_pOwner->GetPosition().z),
        q,                                      // Body の初期ワールド回転
        JPH::EMotionType::Kinematic,            // Static / Kinematic / Dynamic
        Layers::NON_MOVING                      // 衝突レイヤ（フィルタリング用）
    );

    // 生成チェック
    if (JPH::Body* body = bi.CreateBody(settings)) {
        m_BodyID = body->GetID();
        bi.AddBody(m_BodyID, JPH::EActivation::Activate);
    }
    else {
        OutputDebugStringA("Failed to create body!\n");
    }
}
