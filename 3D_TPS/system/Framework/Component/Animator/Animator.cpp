#include "Animator.h"
#include <algorithm>
#include <cmath>
#include <assimp/scene.h>

static float WrapOrClampTime(float tSec, float durSec, bool loop, bool& outFinished)
{
    outFinished = false;

    if (durSec <= 1e-6f)
        return 0.0f;

    if (loop)
    {
        // ループ：0..dur に巻き戻す
        float r = std::fmod(tSec, durSec);
        if (r < 0.0f) r += durSec;
        return r;
    }
    else
    {
        // 非ループ：最後で止める
        if (tSec >= durSec)
        {
            outFinished = true;
            return durSec;
        }
        if (tSec <= 0.0f)
            return 0.0f;

        return tSec;
    }
}

float Animator::GetDurationSec(const aiAnimation* clip)
{
    if (!clip) return 0.0f;

    // aiAnimation::mDuration は ticks
    // mTicksPerSecond が 0 の場合がある
    const double tps = (clip->mTicksPerSecond > 0.0) ? clip->mTicksPerSecond : 25.0;
    const double dur = (tps > 0.0) ? (clip->mDuration / tps) : 0.0;

    return static_cast<float>(dur);
}

float Animator::GetCurrentNormalizedTime() const
{
    float dur = GetCurrentDurationSec();
    if (dur <= 1e-6f) return 0.0f;
    return std::clamp(m_CurrentTimeSec / dur, 0.0f, 1.0f);
}

void Animator::SetCurrentTimeSec(float sec)
{
    if (!m_CurrentClip)
        return;

    const float dur = GetCurrentDurationSec();
    bool finished = false;
    m_CurrentTimeSec = WrapOrClampTime(sec, dur, m_LoopCurrent, finished);
    m_CurrentFinished = finished;
}

void Animator::SetCurrentNormalizedTime(float t01)
{
    if (!m_CurrentClip)
        return;

    const float dur = GetCurrentDurationSec();
    if (dur <= 1e-6f)
    {
        m_CurrentTimeSec = 0.0f;
        m_CurrentFinished = false;
        return;
    }

    t01 = std::clamp(t01, 0.0f, 1.0f);
    SetCurrentTimeSec(t01 * dur);
}

void Animator::SetInitialClip(aiAnimation* clip, float startTimeSec, bool loop)
{
    m_CurrentClip = clip;
    m_LoopCurrent = loop;

    m_NextClip = nullptr;
    m_LoopNext = true;

    m_BlendDuration = 0.0f;
    m_BlendTimer = 0.0f;

    m_CurrentFinished = false;

    SetCurrentTimeSec(startTimeSec);
    m_NextTimeSec = 0.0f;
}

void Animator::ForceSetClip(aiAnimation* clip, float startTimeSec, bool loop)
{
    if (!clip) return;
    SetInitialClip(clip, startTimeSec, loop);
}

void Animator::RequestTransition(aiAnimation* nextClip, float blendDurationSec, bool nextLoop)
{
    if (!nextClip)
        return;

    if (!m_CurrentClip)
    {
        SetInitialClip(nextClip, 0.0f, nextLoop);
        return;
    }

    if (IsBlending() && m_NextClip == nextClip)
        return;

    if (nextClip == m_CurrentClip)
        return;

    if (blendDurationSec <= 0.0f)
    {
        SetInitialClip(nextClip, 0.0f, nextLoop);
        return;
    }

    m_NextClip = nextClip;
    m_LoopNext = nextLoop;

    m_NextTimeSec = 0.0f;
    m_BlendDuration = blendDurationSec;
    m_BlendTimer = 0.0f;
}

void Animator::Update(const float dt)
{
    if (!m_CurrentClip)
        return;

    // dt <= 0 のときは何もしない（停止など）
    if (dt <= 0.0f)
        return;

    // current
    {
        const float dur = GetCurrentDurationSec();
        bool finished = false;
        m_CurrentTimeSec = WrapOrClampTime(m_CurrentTimeSec + dt, dur, m_LoopCurrent, finished);
        m_CurrentFinished = finished;
    }

    if (!IsBlending())
        return;

    // next
    {
        const float durN = GetDurationSec(m_NextClip);
        bool dummy = false;
        m_NextTimeSec = WrapOrClampTime(m_NextTimeSec + dt, durN, m_LoopNext, dummy);
    }

    m_BlendTimer += dt;

    if (m_BlendTimer >= m_BlendDuration)
    {
        // ブレンド完了 → 次を現在に昇格
        m_CurrentClip = m_NextClip;
        m_LoopCurrent = m_LoopNext;

        m_CurrentTimeSec = m_NextTimeSec;

        m_NextClip = nullptr;
        m_NextTimeSec = 0.0f;

        m_BlendDuration = 0.0f;
        m_BlendTimer = 0.0f;

        // 昇格直後の finished 判定を更新
        const float dur = GetCurrentDurationSec();
        bool finished = false;
        m_CurrentTimeSec = WrapOrClampTime(m_CurrentTimeSec, dur, m_LoopCurrent, finished);
        m_CurrentFinished = finished;
    }
}

float Animator::GetBlendAlpha() const
{
    if (!IsBlending())
        return 0.0f;

    float t = m_BlendTimer / m_BlendDuration;
    return std::clamp(t, 0.0f, 1.0f);
}