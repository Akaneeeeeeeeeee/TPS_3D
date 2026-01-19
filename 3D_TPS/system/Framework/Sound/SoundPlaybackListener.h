#pragma once
#include "Framework/Sound/IWorldSoundListener.h"
#include "system/Sound/WorldSoundEvent.h"

class WeatherSystem;
class SoundSystem;
class SoundBus;

class SoundPlaybackListener final : public IWorldSoundListener
{
public:
    void SetBus(SoundBus* bus) { m_pBus = bus; }
    void SetWeatherSystem(WeatherSystem* ws) { m_pWeather = ws; }
    void SetSoundSystem(SoundSystem* ss) { m_pSound = ss; }

    void OnWorldSound(const WorldSoundEvent& ev) override;

private:
    SOUND_LABEL MapLabel(const WorldSoundEvent& ev) const;

private:
    SoundBus* m_pBus = nullptr;     // listenerPosŽæ“¾
    WeatherSystem* m_pWeather = nullptr; // hearingFactor
    SoundSystem* m_pSound = nullptr;   // PlayOneShot
};
