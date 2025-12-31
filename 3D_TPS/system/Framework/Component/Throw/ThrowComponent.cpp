#include "Framework/Component/Throw/ThrowComponent.h"
#include "Framework/ObjectManager/ObjectManager.h"
#include "Framework/GameObject/GameObject.h"
#include "system/LineDrawer.h"

// ThrowActions.cpp 側で提供
std::unique_ptr<IThrowAction> MakeRockThrowAction();
// std::unique_ptr<IThrowAction> MakeCanThrowAction();
// std::unique_ptr<IThrowAction> MakeGrenadeThrowAction();

namespace
{
    constexpr float EPS_SQ = 1e-6f;
    constexpr float DEG2RAD = PI / 180.0f;

    // 「カメラforward」から「投げ方向」を作る（角度を制限）
    static Vector3 BuildThrowDirFromCameraForward(Vector3 camFwd, float minAngleDeg, float maxAngleDeg)
    {
        if (camFwd.LengthSquared() < EPS_SQ)
            return Vector3(0, 0, 1);

        camFwd.Normalize();

        // yaw成分（水平前方）
        Vector3 yawFwd(camFwd.x, 0.0f, camFwd.z);
        if (yawFwd.LengthSquared() < EPS_SQ)
            yawFwd = Vector3(0, 0, 1);
        else
            yawFwd.Normalize();

        // pitch角（上向き角）を取得
        const float horiz = std::sqrt(camFwd.x * camFwd.x + camFwd.z * camFwd.z);
        float pitchRad = std::atan2(camFwd.y, std::max(horiz, 1e-6f)); // 上向き＋

        // pitch を制限（下向き投げすぎ/上向き投げすぎ防止）
        const float minRad = minAngleDeg * DEG2RAD;
        const float maxRad = maxAngleDeg * DEG2RAD;
        pitchRad = std::clamp(pitchRad, minRad, maxRad);

        // 最終方向（速度の向き）
        const Vector3 up(0, 1, 0);
        Vector3 dir = yawFwd * std::cos(pitchRad) + up * std::sin(pitchRad);
        dir.Normalize();
        return dir;
    }


    // 放物線を線分で描く（LineDrawerDraw）
    static void DrawThrowGuide_Line(const Vector3& startPos, const Vector3& startVel)
    {
        // ここは物理の重力と必ず合わせる
        // 「1=1cm」なら -980、「1=1m」なら -9.8 が多い
        const Vector3 g(0.0f, -980.0f, 0.0f);

        constexpr int   SEG = 32;
        constexpr float DT = 0.05f;

        Vector3 p = startPos;
        Vector3 v = startVel;

        for (int i = 0; i < SEG; ++i)
        {
            Vector3 nextP = p + v * DT + g * (0.5f * DT * DT);
            Vector3 nextV = v + g * DT;

            Vector3 d = nextP - p;
            float len = d.Length();
            if (len > 1e-4f)
            {
                d /= len;
                Color col(0.2f, 1.0f, 0.2f, 1.0f); // 色は好みで
                // 太さが未設定だと見えない実装になってる可能性があるので、ここで一度入れる
                SetLineWidth(2.0f);
                LineDrawerDraw(len, p, d, col);
            }

            p = nextP;
            v = nextV;
        }
    }
}


void ThrowComponent::Init()
{
    TryResolveAnim();
    EnsureActionsBuilt();
}

void ThrowComponent::Uninit()
{
    m_anim = nullptr;
    m_current = nullptr;

    m_isAiming = false;
    m_state = State::None;
    m_hasSpawned = false;
    m_elapsed = 0.0f;
    m_cooldown = 0.0f;
}

void ThrowComponent::OnAimStart()
{
    m_isAiming = true;
    m_state = State::Hold;
    m_previewId = ThrowItemId::Rock;

    m_current = nullptr;
    m_hasSpawned = false;
    m_elapsed = 0.0f;

    EnterHold();
}

void ThrowComponent::OnAimEnd()
{
    m_isAiming = false;
    m_state = State::None;

    m_current = nullptr;
    m_hasSpawned = false;
    m_elapsed = 0.0f;
    // cooldown は維持してもいいし、解除してもいい（好み）
}

void ThrowComponent::Throw(ThrowItemId id)
{
    if (!m_isAiming) return;
    if (m_cooldown > 0.0f) return;
    if (m_state != State::Hold) return;

    TryResolveAnim();
    if (!m_anim) return;

    m_current = FindAction(id);
    if (!m_current) return;
    m_previewId = id;

    // hold解除して再生開始
    m_state = State::Throwing;
    m_hasSpawned = false;
    m_elapsed = 0.0f;

    m_anim->SetPlaybackSpeed(1.0f);
}

void ThrowComponent::LateUpdate(float dt)
{
    if (m_cooldown > 0.0f)
        m_cooldown = std::max(0.0f, m_cooldown - dt);

    if (!m_isAiming)
        return;

    TryResolveAnim();
    if (!m_anim) return;

    // 構え中は Throw 側が StoneThrow を主導（Playerの移動アニメと競合させない）
    if (m_anim->GetCurrentClip() != m_anim->GetClipPtr(AnimType::StoneThrow))
    {
        EnterHold();
        return;
    }

    if (m_state == State::Hold)
    {
        ApplyHoldEveryFrame();

        // ガイド表示（最後に選んだ投げ物。無ければRock）
        IThrowAction* preview = FindAction(m_previewId);
        if (!preview) preview = FindAction(ThrowItemId::Rock);

        if (preview && m_pOwner)
        {
            const ThrowTuning& t = preview->Tuning();

            // 生成位置は「水平前方」に出す（上下向きでも手前に出すため）
            Vector3 camFwd = GetAimForward();
            Vector3 yawFwd(camFwd.x, 0.0f, camFwd.z);
            if (yawFwd.LengthSquared() > EPS_SQ) yawFwd.Normalize();
            else yawFwd = Vector3(0, 0, 1);

            const Vector3 pos = m_pOwner->GetPosition()
                + yawFwd * t.spawnForward
                + Vector3(0.0f, t.spawnUp, 0.0f);

            const Vector3 vel = ComputeThrowVelocity(t);

            DrawThrowGuide(pos, vel);
        }
        return;
    }

    if (m_state != State::Throwing)
        return;

    if (!m_current)
        return;

    if (!m_pOwner)
        return;

    ObjectManager* om = m_pOwner->GetObjectManager();
    if (!om)
        return;

    const ThrowTuning& t = m_current->Tuning();

    const float dur = m_anim->GetCurrentDurationSec();
    if (dur <= 1e-6f)
        return;

    // release 時刻（秒）を決める
    const float hold = std::clamp(t.holdNorm, 0.0f, 1.0f);
    const float relN = SanitizeReleaseNorm(t);
    const float releaseSec = std::max(0.0f, (relN - hold) * dur);

    const float prev = m_elapsed;
    m_elapsed += dt;

    if (!m_hasSpawned && prev < releaseSec && m_elapsed >= releaseSec)
    {
        Vector3 camFwd = GetAimForward();

        Vector3 yawFwd(camFwd.x, 0.0f, camFwd.z);
        if (yawFwd.LengthSquared() > EPS_SQ)
        {
            yawFwd.Normalize();
        }
        else
        {
            yawFwd = Vector3(0, 0, 1);
        }

        const Vector3 pos = m_pOwner->GetPosition()
            + yawFwd * t.spawnForward
            + Vector3(0.0f, t.spawnUp, 0.0f);

        // カメラ上下向きを反映した速度
        const Vector3 vel = ComputeThrowVelocity(t);

        ThrowSpawnArgs a{ *m_pOwner, *om, pos, vel };
        m_current->Spawn(a);

        m_hasSpawned = true;
        m_cooldown = std::max(0.0f, t.cooldownSec);
    }

    // 終了で Hold に戻す（StoneThrow の finished が使えるなら優先）
    if (m_anim->IsCurrentFinished() || (m_elapsed >= dur))
    {
        m_state = State::Hold;
        m_hasSpawned = false;
        m_elapsed = 0.0f;
        EnterHold();
    }
}

void ThrowComponent::TryResolveAnim()
{
    if (m_anim) return;
    if (!m_pOwner) return;
    m_anim = m_pOwner->GetComponent<SkinnedAnimationComponent>();
}

void ThrowComponent::EnsureActionsBuilt()
{
    if (m_actionsBuilt) return;
    m_actionsBuilt = true;

    // 「種類→実装」の結び付けはどこかに必ず必要
    // ただし Game::Init から追い出して Throw に閉じ込める
    m_actions[0] = MakeRockThrowAction();
    // m_actions[1] = MakeCanThrowAction();
    // m_actions[2] = MakeGrenadeThrowAction();
}

IThrowAction* ThrowComponent::FindAction(ThrowItemId id)
{
    EnsureActionsBuilt();
    for (auto& a : m_actions)
        if (a && a->Id() == id) return a.get();
    return nullptr;
}

void ThrowComponent::EnterHold()
{
    TryResolveAnim();
    if (!m_anim) return;

    // StoneThrow を最初から（非ループ）
    m_anim->ForceSet(AnimType::StoneThrow, 0.0f, false);

    ApplyHoldEveryFrame();
}

void ThrowComponent::ApplyHoldEveryFrame()
{
    if (!m_anim) return;

    // holdNorm は「今装備中の投げ物」ではなく、
    // 「構え姿勢の固定位置」として共通で良いなら固定値でもOK。
    // ここでは Rock の値を基準にしたいなら m_current を見るように変更。
    // 今回は「構えだけ共通」扱いで 0.20 を固定にしている。
    constexpr float HOLD_NORM_DEFAULT = 0.20f;

    m_anim->SetCurrentNormalizedTime(HOLD_NORM_DEFAULT);
    m_anim->SetPlaybackSpeed(0.0f);
}

float ThrowComponent::SanitizeReleaseNorm(const ThrowTuning& t) const
{
    const float h = std::clamp(t.holdNorm, 0.0f, 1.0f);
    const float r = std::clamp(t.releaseNorm, 0.0f, 1.0f);
    return std::max(h, r);
}

Vector3 ThrowComponent::ComputeThrowVelocity(const ThrowTuning& t) const
{
    Vector3 camFwd = GetAimForward();

    // 投げ角の制限（必要なら ThrowTuning に持たせてもOK）
    constexpr float MIN_ANGLE_DEG = -5.0f;  // 少し下向きは許可
    constexpr float MAX_ANGLE_DEG = 60.0f;  // 上向き上限

    Vector3 dir = BuildThrowDirFromCameraForward(camFwd, MIN_ANGLE_DEG, MAX_ANGLE_DEG);

    // 速さは一定（t.speed を「速さ」にする）
    return dir * t.speed;
}

// ※ カメラの forward を取れるようにする。無ければ owner forward を使う。
Vector3 ThrowComponent::GetAimForward() const
{
    if (!m_pOwner) return Vector3(0, 0, 1);

    // forward = (LookAt - Position) で作る
    if (auto* cam = m_pOwner->GetComponent<CameraComponent>())
    {
        Vector3 f = cam->GetLookAt() - cam->GetPosition();
        if (f.LengthSquared() > EPS_SQ)
        {
            f.Normalize();
            return f;
        }
    }

    // フォールバック：オーナーforward
    Vector3 f = m_pOwner->GetForward();
    if (f.LengthSquared() > EPS_SQ)
    {
        f.Normalize();
        return f;
    }
    return Vector3(0, 0, 1);
}

void ThrowComponent::DrawThrowGuide(const Vector3& startPos, const Vector3& startVel) const
{
    DrawThrowGuide_Line(startPos, startVel);
}
