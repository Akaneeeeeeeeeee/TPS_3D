#pragma once
#include "system/commontypes.h"
#include "system/Sound/WorldSoundEvent.h"

class WeatherSystem;
class SoundSystem;

class WeatherAudioController
{
public:
    void SetWeatherSystem(WeatherSystem* ws) { m_pWeather = ws; }
    void SetSoundSystem(SoundSystem* ss) { m_pSound = ss; }

    void Update(float dt);
	void Uninit(void);

private:
    struct LoopSlot
    {
        SOUND_LABEL label = SOUND_LABEL_MAX;
        float vol = 0.0f;
        float target = 0.0f;
        bool playing = false;
    };

    void EnsureLoop(LoopSlot& s, SOUND_LABEL label);
    void StopIfSilent(LoopSlot& s);
    void ComputeTarget(SOUND_LABEL& outLabel, float& outVol) const;
    void UpdateThunder(float dt, float intensity01);

private:
    WeatherSystem* m_pWeather = nullptr;
    SoundSystem* m_pSound = nullptr;

    LoopSlot m_A{};
    LoopSlot m_B{};
    float m_FadeSpeed = 1.5f;

    float m_ThunderTimer = 0.0f;
};
