#include "StreetLight.h"
#include "Framework/Component/DayNightObserver/DayNightObserver.h"

void StreetLight::Awake()
{
    m_Spot = AddComponent<SpotLightComponent>("Spot");
    m_DayNight = AddComponent<DayNightObserverComponent>("DayNight");

    if (m_DayNight)
        m_DayNight->SetReceiver(this);
}

void StreetLight::Start()
{
    ApplyPendingToSpot();
    RefreshLighting(); // 現在の m_IsNight で最終点灯を決定
}

void StreetLight::Uninit()
{
    // StreetLightが先に死んでも転送されないように
    if (m_DayNight) m_DayNight->SetReceiver(nullptr);

    m_DayNight = nullptr;
    m_Spot = nullptr;
	GameObject::Uninit();
}

void StreetLight::OnDayNightChanged(bool isNight)
{
    m_IsNight = isNight;
    RefreshLighting();
}

void StreetLight::SetNightOnly(bool e)
{
    m_Pending.nightOnly = e;
    RefreshLighting();
}

void StreetLight::SetEnabled(bool e)
{
    m_Pending.enabled = e;
    RefreshLighting();
}

void StreetLight::SetColor(const Color& c)
{
    m_Pending.color = c;
    if (m_Spot) m_Spot->SetColor(c);
}

void StreetLight::SetIntensity(float i)
{
    m_Pending.intensity = i;
    if (m_Spot) m_Spot->SetIntensity(i);
}

void StreetLight::SetGroundCircle(float groundRadius, float groundY, float topRadiusMin, float innerRatio)
{
    m_Pending.useGroundFit = true;
    m_Pending.groundRadius = groundRadius;
    m_Pending.groundY = groundY;
    m_Pending.topRadiusMin = topRadiusMin;
    m_Pending.innerRatio = innerRatio;

    m_Pending.aimMode = SpotLightComponent::AimMode::BelowY;

    if (m_Spot)
        ApplyPendingToSpot();
}

void StreetLight::ApplyPendingToSpot()
{
    if (!m_Spot) return;

    // ★ここで SetEnabled はしない（最終決定は RefreshLighting のみ）
    m_Spot->SetColor(m_Pending.color);
    m_Spot->SetIntensity(m_Pending.intensity);

    if (m_Pending.aimMode == SpotLightComponent::AimMode::BelowY)
        m_Spot->SetAimBelowY(m_Pending.groundY);
    else
        m_Spot->SetAimWorldDown();

    if (m_Pending.useGroundFit)
    {
        m_Spot->FitToGroundCircle(
            m_Pending.groundY,
            m_Pending.groundRadius,
            m_Pending.topRadiusMin,
            m_Pending.innerRatio,
            /*nearMinAxis=*/10.0f,
            /*rangeExtraAxis=*/0.0f
        );
    }
}

void StreetLight::RefreshLighting()
{
    if (!m_Spot) return;

    bool wantLit = m_Pending.enabled;

    if (m_Pending.nightOnly)
        wantLit = wantLit && m_IsNight;

    if (wantLit == m_RuntimeLit) return;
    m_RuntimeLit = wantLit;

    m_Spot->SetEnabled(m_RuntimeLit);
}
