#include "Framework/Sound/SoundPlaybackListener.h"
#include "Framework/Sound/SoundBus.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/SoundManager/SoundSystem.h"
#include <algorithm>

SOUND_LABEL SoundPlaybackListener::MapLabel(const WorldSoundEvent& ev) const
{
    if (ev.PlayLabel != SOUND_LABEL_MAX) return ev.PlayLabel;

    switch (ev.Type)
    {
    case SoundType::StoneImpact: return SE_STONE;
    case SoundType::Footstep:    return SE_WALKING_NORMAL;
    default:                     return SOUND_LABEL_MAX;
    }
}

void SoundPlaybackListener::OnWorldSound(const WorldSoundEvent& ev)
{
    if (!m_pBus || !m_pSound) return;

    const SOUND_LABEL label = MapLabel(ev);
    if (label == SOUND_LABEL_MAX) return;

    float hearing = 1.0f;
    if (m_pWeather) hearing = m_pWeather->GetHearingFactor();

    const float effectiveRadius = ev.Radius * hearing;
    if (effectiveRadius <= 1e-6f) return;

    // 距離基準点（ここが「プレイヤー位置」になっていれば音量で距離が分かる）
    const Vector3 listener = m_pBus->GetListenerPos();

    const float dist = (ev.Position - listener).Length();
    if (dist >= effectiveRadius) return;

    float att = 1.0f - (dist / effectiveRadius);
    att = std::clamp(att, 0.0f, 1.0f);

    // ---- 種別倍率（EmitterKind） ----
    float kindMul = 1.0f;
    if (ev.Emitter == SoundEmitterKind::Player && ev.Type == SoundType::Footstep)
        kindMul = 0.9f;          // 自分の足音を少し小さく
    else if (ev.Emitter == SoundEmitterKind::PlayerItem)
        kindMul = 1.2f;          // 投げた石などは少し大きめ

    float vol = ev.Volume * att * hearing * kindMul;
    if (vol <= 0.0f) return;

    m_pSound->PlayOneShot(label, vol);
}
