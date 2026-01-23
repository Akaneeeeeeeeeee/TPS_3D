#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/EngineSystem/EngineSystem.h"
#include "Framework/Component/Throw/IThrowEventListener.h"


class ThrowAudioComponent final : public IComponent, public IThrowEventListener
{
public:
    DECLARE_COMPONENT_TYPE(ThrowAudioComponent, IComponent)
    void Attach(EngineServices& ctx) override { m_Sound = &ctx.sound; }
    void Detach() override { m_Sound = nullptr; }

    void Init() override {}
    void Update(const float) override {}
    void Uninit() override {}

    void OnThrowReleased(ThrowItemId id) override
    {
        if (!m_Sound) return;
        // ‹——£–³ŠÖŒW‚ÅŠmŽÀ‚É
        m_Sound->PlayUIOneShot(SE_THROW, 1.0f);
    }

private:
    SoundHub* m_Sound = nullptr;
};
