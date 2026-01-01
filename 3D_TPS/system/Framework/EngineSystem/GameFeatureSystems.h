#pragma once
#include "system/Framework/EngineSystem/EngineSystem.h"
#include "system/Sound/SoundWaveVisualizer.h"

class GameFeatureSystems
{
public:
    void Init(EngineServices& svc)
    {
        // 可視化が天候の影響を受けるならここで渡す
        m_SoundWave.SetWeatherSystem(&svc.weather);

        // 基盤のSoundSystemに「見た目」を購読させる
        //svc.sound.RegisterListener(&m_SoundWave);
    }

    void Update(float dt)
    {
        m_SoundWave.Update(dt);
    }

    void DrawWorld()
    {
        m_SoundWave.DrawWorld();
    }

private:
    SoundWaveVisualizer m_SoundWave; // 旧SoundWaveVisualizer（非Singleton）
};
