#pragma once
#include <vector>
#include "commontypes.h"
#include "system/Sound/WorldSoundEvent.h"
#include "Framework/Sound/IWorldSoundListener.h"

// ëOï˚êÈåæ
class SoundSystem;
class WeatherSystem;

class SoundWaveVisualizer : public IWorldSoundListener
{
public:
    SoundWaveVisualizer() = default;

    void Update(float dt);
    void DrawWorld();
    void DrawUI();

    void OnEmit(const WorldSoundEvent& ev);

    void SetMaxLoudness(float v) { m_MaxLoudness = v; }
    void SetWeatherSystem(WeatherSystem* ws) { m_pWeather = ws; }

    void OnWorldSound(const WorldSoundEvent& ev) override;

private:
    struct Wave
    {
        Vector3 center;
        float   maxRadius = 0.0f;
        float   currentRadius = 0.0f;
        float   lifeTime = 0.5f;
        float   elapsed = 0.0f;
        float   loudness = 1.0f;
        Color   baseColor = Color(1, 1, 1, 1);
    };

    std::vector<Wave> m_Waves;

    float m_MaxLoudness = 1.0f;
    WeatherSystem* m_pWeather = nullptr;
};