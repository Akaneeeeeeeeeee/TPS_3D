#include "SpotLightComponent.h"
#include "Framework/EngineSystem/EngineSystem.h"
#include "Framework/GameObject/GameObject.h"
#include <algorithm>
#include <cmath>

// 角度→ラジアン
static float DegToRad(float deg) { return deg * (PI / 180.0f); }
static float RadToDeg(float rad) { return rad * (180.0f / PI); }

// 0..1 に丸める
static float Saturate(float x) { return std::clamp(x, 0.0f, 1.0f); }

void SpotLightComponent::SetNear(float n)
{
    // 0以上、range以下にクランプ
    m_Near = std::clamp(n, 0.0f, m_Range);
}

void SpotLightComponent::Attach(EngineServices& context)
{
    m_System = &context.light;
    m_System->Register(this);
}

void SpotLightComponent::Detach()
{
    if (m_System)
    {
        m_System->Unregister(this);
        m_System = nullptr;
    }
}

void SpotLightComponent::SetAnglesDeg(float innerDeg, float outerDeg)
{
    if (innerDeg > outerDeg) std::swap(innerDeg, outerDeg);

    innerDeg = std::clamp(innerDeg, 0.1f, 179.0f);
    outerDeg = std::clamp(outerDeg, 0.1f, 179.0f);

    const float innerRad = DegToRad(innerDeg);
    const float outerRad = DegToRad(outerDeg);

    // 「半角」を使う流儀：cos(θ/2)
    m_InnerCos = std::cos(innerRad * 0.5f);
    m_OuterCos = std::cos(outerRad * 0.5f);

    // もし inner と outer が極端に近いと 0 除算になるので最低差を作る
    if (m_InnerCos < m_OuterCos + 1e-6f)
        m_InnerCos = m_OuterCos + 1e-6f;
}

void SpotLightComponent::SetTopRadius(float topRadius)
{
    // 「topRadius = tan(outerHalfAngle) * near」 なので near を逆算する
    // outerHalfAngle = acos(m_OuterCos)
    const float outerHalf = std::acos(std::clamp(m_OuterCos, -1.0f, 1.0f));
    const float tanHalf = std::tan(outerHalf);

    if (tanHalf < 1e-6f)
    {
        // ほぼ角度0：円錐台にならないので near=0 扱い
        SetNear(0.0f);
        return;
    }

    float nearDist = topRadius / tanHalf;
    SetNear(nearDist); // 0..range にクランプされる
}

Vector3 SpotLightComponent::GetLightPos() const
{
    if (!m_pOwner) return Vector3::Zero;
    return m_pOwner->GetPosition();
}

Vector3 SpotLightComponent::GetLightDir() const
{
    // 正規化済みを返す
    if (!m_pOwner) return Vector3(0, -1, 0);

    const Vector3 pos = m_pOwner->GetPosition();

    Vector3 dir = Vector3(0, 0, -1);

    switch (m_AimMode)
    {
    case AimMode::OwnerForward:
        dir = m_pOwner->GetForward();
        break;

    case AimMode::WorldDown:
        dir = Vector3(0, -1, 0);
        break;

    case AimMode::BelowY:
    {
        // (x, groundY, z) を狙う
        Vector3 target(pos.x, m_GroundY, pos.z);
        dir = target - pos;
        break;
    }
    default:
        dir = Vector3(0, -1, 0);
        break;
    }

    if (dir.LengthSquared() < 1e-6f)
        dir = Vector3(0, -1, 0);
    else
        dir.Normalize();

    return dir;
}

bool SpotLightComponent::BuildGPU(SpotLightGPU& out) const
{
    if (!m_Enabled) return false;
    if (!m_pOwner) return false;

    const Vector3 pos = GetLightPos();
    const Vector3 dir = GetLightDir();

    out.Position = Vector4(pos.x, pos.y, pos.z, 1.0f);
    out.Direction = Vector4(dir.x, dir.y, dir.z, 0.0f);
    out.Color = Vector4(m_Color.x, m_Color.y, m_Color.z, 1.0f);

    // 既存の並びを維持（描画用）
    out.Params1 = Vector4(m_Range, m_InnerCos, m_OuterCos, m_Intensity);

    // ここはあなたの SpotLightGPU 定義次第。
    // 「near を描画でも使う」なら Params2 の空きに入れるのが簡単。
    // 例：Params2 = (enabled, near, 0, 0)
    out.Params2 = Vector4(1.0f, m_Near, 0.0f, 0.0f);

    return true;
}

float SpotLightComponent::ComputeInfluence01(const Vector3& worldPos) const
{
    if (!m_Enabled) return 0.0f;
    if (!m_pOwner)  return 0.0f;
    if (m_Range <= 1e-6f) return 0.0f;

    // near は range を超えないように（SetRange 後の安全策）
    const float nearD = std::clamp(m_Near, 0.0f, m_Range);

    const Vector3 L = GetLightPos();
    const Vector3 D = GetLightDir(); // 正規化済み

    Vector3 v = worldPos - L;
    const float dist = v.Length();

    // 円錐台：near より手前は 0、range より奥も 0
    if (dist <= nearD + 1e-4f) return 0.0f;
    if (dist > m_Range)       return 0.0f;

    // 角度判定（outer）
    const Vector3 dirTo = v / dist;
    const float cosAng = dirTo.Dot(D);

    if (cosAng < m_OuterCos)
        return 0.0f;

    // 角度減衰：outer→inner を 0..1 に
    const float denomA = (m_InnerCos - m_OuterCos);
    float angle01 = (denomA > 1e-6f) ? (cosAng - m_OuterCos) / denomA : 1.0f;
    angle01 = Saturate(angle01);

    // 距離減衰：near→range を 1..0 に（near が「上面」）
    const float denomD = (m_Range - nearD);
    float dist01 = (denomD > 1e-6f) ? 1.0f - (dist - nearD) / denomD : 0.0f;
    dist01 = Saturate(dist01);

    // 「上面が強い」＝ near 付近を強くしたいなら指数をかける（好み）
    // dist01 = std::pow(dist01, 0.7f);  // 0.7 だと近いほどさらに強く（例）
    // angle01 = std::pow(angle01, 1.2f); // 中心寄りを強く（例）

    // 0..1 で返す（intensity は LightSystem 側で倍率にするのが扱いやすい）
    return angle01 * dist01;
}


void SpotLightComponent::FitToGroundCircle(float groundY, float groundRadius,
    float topRadiusMin,
    float innerRatio,
    float nearMinAxis,
    float rangeExtraAxis)
{
    if (!m_pOwner) return;

    groundRadius = std::max(groundRadius, 0.0f);
    topRadiusMin = std::max(topRadiusMin, 0.0f);
    innerRatio = std::clamp(innerRatio, 0.0f, 1.0f);
    nearMinAxis = std::max(nearMinAxis, 0.0f);
    rangeExtraAxis = std::max(rangeExtraAxis, 0.0f);

    const Vector3 pos = m_pOwner->GetPosition();
    const float H = pos.y - groundY;
    if (H <= 1e-3f) return;

    // ① 角度（地面の半径 / 高さ から outer を決める）
    const float outerHalf = std::atan2(groundRadius, H);
    float outerDeg = RadToDeg(outerHalf * 2.0f);

    const float innerR = groundRadius * innerRatio;
    const float innerHalf = std::atan2(innerR, H);
    float innerDeg = RadToDeg(innerHalf * 2.0f);
    if (innerDeg > outerDeg) innerDeg = outerDeg;

    // ② range：地面の円周まで届く直線距離（重要）
    const float distToEdge = std::sqrt(H * H + groundRadius * groundRadius);
    const float rangeDist = distToEdge + rangeExtraAxis;
    SetRange(rangeDist);

    // ③ near：上面口径(topRadiusMin)から逆算（axis距離でOK）
    const float tanOuter = std::tan(outerHalf);
    float nearAxis = 0.0f;
    if (tanOuter > 1e-6f)
        nearAxis = topRadiusMin / tanOuter;

    nearAxis = std::max(nearAxis, nearMinAxis);

    // near < range を保証
    if (nearAxis >= m_Range)
        nearAxis = m_Range * 0.9f;

    SetAnglesDeg(innerDeg, outerDeg);
    SetNear(nearAxis);
}
