#include "SearchLightControllerComponent.h"

#include "Framework/GameObject/GameObject.h"
#include "Framework/Component/Light/SpotLightComponent.h"

void SearchLightControllerComponent::Init()
{
    m_yawLocalDeg = 0.0f;
    m_dirSign = 1.0f;

    // “ƒ‚Ì‰ŠúŒü‚«‚ğŠî€‚ÉñU‚è‚µ‚½‚¢‚Ì‚Å baseYaw ‚ğæ‚é
    if (m_pOwner)
    {
        Vector3 f = m_pOwner->GetForward();
        f.y = 0.0f;
        if (f.LengthSquared() > 1e-6f) f.Normalize();
        else f = Vector3(0, 0, -1);

        // ‚ ‚È‚½‚Ì Player ‚Ì—¬‹Viatan2(-x,-z)j‚Æ‘µ‚¦‚é
        const float yawRad = std::atan2(-f.x, -f.z);
        m_baseYawDeg = yawRad * (180.0f / PI);
    }

    ApplyToSpot();
}

void SearchLightControllerComponent::Update(const float dt)
{
    if (!m_spot) return;

    if (!m_powered)
    {
        m_spot->SetEnabled(false);
        return;
    }

    m_spot->SetEnabled(true);

    UpdatePatrol(dt);
    ApplyToSpot();
}

void SearchLightControllerComponent::UpdatePatrol(float dt)
{
    m_yawLocalDeg += m_dirSign * yawSpeedDeg * dt;

    if (m_yawLocalDeg > sweepMaxDeg)
    {
        m_yawLocalDeg = sweepMaxDeg - (m_yawLocalDeg - sweepMaxDeg);
        m_dirSign = -1.0f;
    }
    else if (m_yawLocalDeg < sweepMinDeg)
    {
        m_yawLocalDeg = sweepMinDeg + (sweepMinDeg - m_yawLocalDeg);
        m_dirSign = 1.0f;
    }
}

void SearchLightControllerComponent::ApplyToSpot()
{
    if (!m_spot) return;

    // base + local ‚Å g“ƒ‚ÌŒü‚«Šî€‚ÌñU‚èh
    const float yawDeg = m_baseYawDeg + m_yawLocalDeg;
    const Vector3 dir = MakeDirFromYawPitchDeg(yawDeg, pitchDeg);

    m_spot->SetAimManualDir(dir);
}

Vector3 SearchLightControllerComponent::MakeDirFromYawPitchDeg(float yawDeg, float pitchDeg_) const
{
    const float yaw = yawDeg * (PI / 180.0f);
    const float pit = pitchDeg_ * (PI / 180.0f);

    const float ce = std::cos(pit);
    const float se = std::sin(pit);
    const float ca = std::cos(yaw);
    const float sa = std::sin(yaw);

    // yaw=0 ‚Å (0,0,-1) ‚ğŒü‚­i‚ ‚È‚½‚Ì®‚Æ“¯Œnj
    Vector3 d(-sa * ce, se, -ca * ce);

    if (d.LengthSquared() < 1e-6f) return Vector3(0, 0, -1);
    d.Normalize();
    return d;
}