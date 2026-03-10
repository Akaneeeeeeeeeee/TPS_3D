#include "Framework/Sound/WeatherAudioController.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/SoundSystem/SoundSystem.h"
#include <algorithm>

static float Approach(float cur, float target, float delta)
{
    if (cur < target) return std::min(cur + delta, target);
    return std::max(cur - delta, target);
}

void WeatherAudioController::EnsureLoop(LoopSlot& s, SOUND_LABEL label)
{
    if (!m_pSound) return;

    if (s.label == label && s.playing) return;

    if (s.playing && s.label != SOUND_LABEL_MAX)
    {
        m_pSound->StopLoop(s.label);
        s.playing = false;
    }

    s.label = label;
    s.vol = 0.0f;
    s.target = 0.0f;
    s.playing = false;

    if (label != SOUND_LABEL_MAX)
    {
        m_pSound->PlayLoop(label, 0.0f);
        s.playing = true;
    }
}

void WeatherAudioController::StopIfSilent(LoopSlot& s)
{
    if (!m_pSound) return;
    if (!s.playing || s.label == SOUND_LABEL_MAX) return;

    if (s.vol <= 0.001f && s.target <= 0.001f)
    {
        m_pSound->StopLoop(s.label);
        s.playing = false;
        s.label = SOUND_LABEL_MAX;
        s.vol = 0.0f;
        s.target = 0.0f;
    }
}

void WeatherAudioController::ComputeTarget(SOUND_LABEL& outLabel, float& outVol) const
{
    outLabel = SOUND_LABEL_MAX;
    outVol = 0.0f;

    if (!m_pWeather) return;

    const auto wt = m_pWeather->GetWeather();

    const float rain01 = m_pWeather->GetRainStrength01();
    const float sand01 = m_pWeather->GetSandStrength01();

    switch (wt)
    {
    case WeatherType::Clear:
        outLabel = SOUND_LABEL_MAX;
        outVol = 0.0f;
        break;

    case WeatherType::LightRain:
        outLabel = SE_LIGHTRAIN;
        outVol = rain01;
        break;

    case WeatherType::HeavyRain:
        outLabel = SE_HEAVYRAIN;
        outVol = rain01;
        break;

    case WeatherType::Sandstorm:
        outLabel = SE_SANDSTORM;
        outVol = sand01;
        break;

    default:
        break;
    }

    outVol = std::clamp(outVol, 0.0f, 1.0f);
}

void WeatherAudioController::UpdateThunder(float dt, float intensity01)
{
    if (!m_pSound) return;

    if (intensity01 <= 0.01f)
    {
        m_ThunderTimer = 0.0f;
        return;
    }

    m_ThunderTimer -= dt;
    if (m_ThunderTimer > 0.0f) return;

    // 強いほど頻度が上がる（目安）
    const float minT = 2.0f;
    const float maxT = 10.0f;
    const float next = maxT - (maxT - minT) * intensity01;
    m_ThunderTimer = next;

    const float vol = 0.6f + 0.4f * intensity01;
    m_pSound->PlayOneShot(SE_THUNDER, vol);
}

void WeatherAudioController::Update(float dt)
{
    if (!m_pWeather || !m_pSound) return;

    SOUND_LABEL wantLabel;
    float wantVol;
    ComputeTarget(wantLabel, wantVol);

    LoopSlot* active = nullptr;
    LoopSlot* other = nullptr;

    if (m_A.label == wantLabel) { active = &m_A; other = &m_B; }
    else if (m_B.label == wantLabel) { active = &m_B; other = &m_A; }
    else
    {
        LoopSlot* slot = (!m_A.playing ? &m_A : (!m_B.playing ? &m_B : &m_B));
        EnsureLoop(*slot, wantLabel);
        active = slot;
        other = (slot == &m_A) ? &m_B : &m_A;
    }

    if (active) active->target = (wantLabel == SOUND_LABEL_MAX) ? 0.0f : wantVol;
    if (other)  other->target = 0.0f;

    const float dv = m_FadeSpeed * dt;

    for (LoopSlot* s : { &m_A, &m_B })
    {
        if (!s->playing || s->label == SOUND_LABEL_MAX) continue;

        s->vol = Approach(s->vol, s->target, dv);
        m_pSound->SetLoopVolume(s->label, s->vol);
        StopIfSilent(*s);
    }

    // 雷：雨の強さに比例（
    UpdateThunder(dt, m_pWeather->GetRainStrength01());
}

void WeatherAudioController::Uninit(void)
{
    if (!m_pSound) return;

    // ループを止める（現在鳴ってる可能性があるもの全部）
    if (m_A.playing && m_A.label != SOUND_LABEL_MAX) m_pSound->StopLoop(m_A.label);
    if (m_B.playing && m_B.label != SOUND_LABEL_MAX) m_pSound->StopLoop(m_B.label);

    m_A = LoopSlot{};
    m_B = LoopSlot{};
    m_ThunderTimer = 0.0f;
}