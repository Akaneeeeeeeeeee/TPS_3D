#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/SoundSystem/SoundSystem.h"
#include "Framework/EngineSystem/EngineSystem.h"

class CountdownSoundComponent final : public IComponent
{
public:
    DECLARE_COMPONENT_TYPE(CountdownSoundComponent, IComponent)

    CountdownSoundComponent(float* remainSec, float warnSec)
        : m_Remain(remainSec), m_Warn(warnSec) {
    }

    void Attach(EngineServices& ctx) override { m_Sound = &ctx.sound; }
    void Detach() override
    {
        if (m_Sound) m_Sound->StopUILoop(SE_COUNTDOWN);
        m_Sound = nullptr;
        m_Playing = false;
    }

    void Update(const float /*dt*/) override
    {
        if (!m_Sound || !m_Remain) return;

        // ポーズ中は鳴らさない
        if (Time::GetInstance().GetTimeScale() <= 0.0001f)
        {
            if (m_Playing) { m_Sound->StopUILoop(SE_COUNTDOWN); m_Playing = false; }
            return;
        }

        const float r = *m_Remain;

        if (r <= 0.0f || r > m_Warn)
        {
            if (m_Playing) { m_Sound->StopUILoop(SE_COUNTDOWN); m_Playing = false; }
            return;
        }

        // 残りが少ないほど少し大きく
        float t = 1.0f - (r / m_Warn);
        t = std::clamp(t, 0.0f, 1.0f);
        float vol = 0.35f + 0.55f * t;

        m_Sound->StartUILoop(SE_COUNTDOWN, vol);
        m_Playing = true;
    }

    void Init() override {}
    void Uninit() override {}

private:
    SoundHub* m_Sound = nullptr;
    float* m_Remain = nullptr;
    float  m_Warn = 60.0f;
    bool   m_Playing = false;
};
