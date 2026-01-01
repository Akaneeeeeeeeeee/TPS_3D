#include "SphereCollider.h"
#include "Framework/GameObject/GameObject.h"

void SphereCollider::Attach(EngineServices& context)
{
    PhysicsComponent::Attach(context);

    // 半径未設定ならスケールから推定（任意）
    if (m_Radius <= 0.0f && m_pOwner)
    {
        Vector3 s = m_pOwner->GetScale();
        const float sx = std::abs(s.x);
        const float sy = std::abs(s.y);
        const float sz = std::abs(s.z);
        const float maxS = std::max(sx, std::max(sy, sz));
        m_Radius = 0.5f * maxS;
    }
}

void SphereCollider::Detach(void)
{
    m_Shape = nullptr;
    PhysicsComponent::Detach();
}

void SphereCollider::Init(void)
{
    // 半径が0だとShape作れないので何もしない
    if (m_Radius <= 0.0f) { return; }

    float r = m_Radius;
    //if (m_pOwner)
    //{
    //    Vector3 s = m_pOwner->GetScale();
    //    float k = std::max({ s.x, s.y, s.z });   // 安全側（必ず覆う）
    //    r *= k;
    //}

    // Shapeだけ作る（Bodyは作らない）
    m_Shape = new JPH::SphereShape(r);
}

void SphereCollider::Uninit(void)
{
    m_Shape = nullptr;
}
