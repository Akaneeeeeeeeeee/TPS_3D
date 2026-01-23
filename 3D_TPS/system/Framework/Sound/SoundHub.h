#pragma once
#include "Framework/Sound/SoundBus.h"
#include "Framework/SoundManager/SoundSystem.h"
#include "Framework/Sound/SoundPlaybackListener.h"
#include "Framework/Sound/WeatherAudioController.h"
#include "system/Sound/SoundWaveVisualizer.h"

class WeatherSystem;


class SoundHub
{
public:
    void Init(WeatherSystem* weather);
    void Uninit(void);

    void BeginFrame(float dt);
    void UpdateFrame(float dt, const Vector3& listenerPos); // listener更新 + 天候音更新
    void EndFrame(float dt);                                // Dispatch + 回収 + 可視化寿命更新

    // ---- World（距離あり）----
    void Emit(const WorldSoundEvent& ev) { m_Bus.Emit(ev); }
    void RegisterListener(IWorldSoundListener* l) { m_Bus.RegisterListener(l); }
    void UnregisterListener(IWorldSoundListener* l) { m_Bus.UnregisterListener(l); }

    // ---- UI/演出（距離なし）----
    void PlayUIOneShot(SOUND_LABEL label, float volume01 = 1.0f)
    {
        m_System.PlayOneShot(label, std::clamp(volume01, 0.0f, 1.0f));
    }

    // ループは Start/Stop/Set を用意（PlayLoopを直接使わない）
    void StartUILoop(SOUND_LABEL label, float volume01 = 1.0f);
    void StopUILoop(SOUND_LABEL label);
    void SetUILoopVolume(SOUND_LABEL label, float volume01);

    // 可視化
    void DrawWorldSound() { m_Viz.DrawWorld(); }

    // コンポーネントから欲しい場合の参照（最小）
    SoundBus& Bus() { return m_Bus; } // 使わなくてもOK

private:
    bool IsValid(SOUND_LABEL l) const { return 0 <= (int)l && (int)l < (int)SOUND_LABEL_MAX; }

private:
    SoundSystem           m_System;
    SoundBus              m_Bus;
    SoundPlaybackListener m_Playback;
    WeatherAudioController m_WeatherAudio;
    SoundWaveVisualizer   m_Viz;

    std::array<bool, SOUND_LABEL_MAX> m_Looping{};  // UIループの再スタート防止
    WeatherSystem* m_pWeather = nullptr;
    float m_LastDt = 0.0f;
};
