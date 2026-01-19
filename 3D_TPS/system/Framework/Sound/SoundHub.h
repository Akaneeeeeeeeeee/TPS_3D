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

    // 発生側はこれだけ使えば良い
    void Emit(const WorldSoundEvent& ev) { m_Bus.Emit(ev); }

    // リスナー登録（EnemyHearingなどがAttachで使う）
    void RegisterListener(IWorldSoundListener* l) { m_Bus.RegisterListener(l); }
    void UnregisterListener(IWorldSoundListener* l) { m_Bus.UnregisterListener(l); }

    // 可視化
    void DrawWorldSound() { m_Viz.DrawWorld(); }

    // コンポーネントから欲しい場合の参照（最小）
    SoundBus& Bus() { return m_Bus; } // 使わなくてもOK

private:
    SoundSystem           m_System;
    SoundBus              m_Bus;
    SoundPlaybackListener m_Playback;
    WeatherAudioController m_WeatherAudio;
    SoundWaveVisualizer   m_Viz;

    WeatherSystem* m_pWeather = nullptr;
    float m_LastDt = 0.0f;
};
