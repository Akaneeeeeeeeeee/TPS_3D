#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/Time/Time.h"

class SoundHub;

class SlowMoSoundComponent final : public IComponent
{
public:
    DECLARE_COMPONENT_TYPE(SlowMoSoundComponent, IComponent)

    void Attach(EngineServices& ctx) override { m_Sound = &ctx.sound; }
    void Detach() override { m_Sound = nullptr; }

    void Init() override { m_WasSlow = IsSlowMo(); }
    void Update(const float /*dt*/) override
    {
        if (!m_Sound) return;

        const bool slow = IsSlowMo();

        // 通常→スロー
        if (!m_WasSlow && slow)
            m_Sound->PlayUIOneShot(SE_STARTSLOWMOTION, 1.0f);

        // スロー→通常
        else if (m_WasSlow && !slow)
            m_Sound->PlayUIOneShot(SE_ENDSLOWMOTION, 1.0f);

        m_WasSlow = slow;
    }

    void Uninit() override {}

private:
    bool IsSlowMo() const
    {
        const float ts = Time::GetInstance().GetTimeScale();
        // 0はポーズ扱いで除外（ポーズで誤鳴りしない）
        return (ts > 0.0001f && ts < 0.999f);
    }

private:
    SoundHub* m_Sound = nullptr;
    bool m_WasSlow = false;
};