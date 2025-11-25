#include "Animator.h"
#include <algorithm>

void Animator::SetInitialClip(aiAnimation* clip, float startTimeSec)
{
    m_CurrentClip = clip;
    m_CurrentTimeSec = startTimeSec;

    m_NextClip = nullptr;
    m_NextTimeSec = 0.0f;

    m_BlendDuration = 0.0f;
    m_BlendTimer = 0.0f;
}

void Animator::RequestTransition(aiAnimation* nextClip, float blendDurationSec)
{
    // 無効な指定なら無視
    if (!nextClip)
        return;

    // まだ何も再生していない → そのままセット
    if (!m_CurrentClip)
    {
        SetInitialClip(nextClip, 0.0f);
        return;
    }

    // すでに同じクリップへブレンド中なら、リセットしない
    if (IsBlending() && m_NextClip == nextClip)
    {
        return;
    }

    // 同じクリップへの遷移なら、ブレンド不要として無視
    if (nextClip == m_CurrentClip)
    {
        return;
    }

    // ブレンド時間が 0 以下なら即切り替え
    if (blendDurationSec <= 0.0f)
    {
        m_CurrentClip = nextClip;
        m_CurrentTimeSec = 0.0f;

        m_NextClip = nullptr;
        m_NextTimeSec = 0.0f;
        m_BlendDuration = 0.0f;
        m_BlendTimer = 0.0f;
        return;
    }

    // 正常なブレンド開始
    m_NextClip = nextClip;
    m_NextTimeSec = 0.0f;           // ブレンド先は先頭から再生
    m_BlendDuration = blendDurationSec;
    m_BlendTimer = 0.0f;
}

void Animator::Update(const float dt)
{
    if (!m_CurrentClip)
        return;

    // 現在アニメの再生時間 [秒] を進める
    m_CurrentTimeSec += dt;

    if (!IsBlending())
    {
        // 単一再生。
        // ループ処理は CAnimationMeshBlender 側で
        //   f = frame % keyCount;
        // として扱う前提ならここでは何もしなくてよい。
        return;
    }

    // ブレンド中なら次側の時間も進める
    m_NextTimeSec += dt;
    m_BlendTimer += dt;

    if (m_BlendTimer >= m_BlendDuration)
    {
        // ブレンド完了 → 次を現在に昇格
        m_CurrentClip = m_NextClip;
        m_CurrentTimeSec = m_NextTimeSec;

        m_NextClip = nullptr;
        m_NextTimeSec = 0.0f;
        m_BlendDuration = 0.0f;
        m_BlendTimer = 0.0f;
    }
}

float Animator::GetBlendAlpha() const
{
    if (!IsBlending())
        return 0.0f;

    float t = m_BlendTimer / m_BlendDuration;
    return std::clamp(t, 0.0f, 1.0f);
}
