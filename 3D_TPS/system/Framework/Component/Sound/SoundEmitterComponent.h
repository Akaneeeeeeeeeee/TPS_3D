#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Sound/WorldSoundEvent.h"


class SoundHub;

class SoundEmitterComponent final : public IComponent
{
public:
    DECLARE_COMPONENT_TYPE(SoundEmitterComponent, IComponent)

    void Attach(EngineServices& ctx) override;
    void Detach(void) override;

	void Init(void) override {}
	void Update(const float deltatime) override {}
	void Uninit(void) override {}

    void EmitSound(const WorldSoundEvent& ev);

    void EmitSound(const Vector3& pos, SoundType type,
        float loudness, float radius, float volume,
        SOUND_LABEL playLabel = SOUND_LABEL_MAX);

    void PlayUIOneShot(SOUND_LABEL label, float volume01 = 1.0f);

private:
    SoundHub* m_pSound = nullptr;
};