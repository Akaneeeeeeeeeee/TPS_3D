#include "SoundEmitterComponent.h"
#include "Framework/GameObject/GameObject.h"


void SoundEmitterComponent::Attach(EngineServices& ctx)
{
    m_pSound = &ctx.sound;
}

void SoundEmitterComponent::Detach()
{
    m_pSound = nullptr;
}

void SoundEmitterComponent::EmitSound(const WorldSoundEvent& ev)
{
    if (!m_pSound) return;
    m_pSound->Emit(ev);
}

void SoundEmitterComponent::PlayUIOneShot(SOUND_LABEL label, float volume01)
{
    if (!m_pSound) return;
    m_pSound->PlayUIOneShot(label, volume01);
}

void SoundEmitterComponent::EmitSound(const Vector3& pos, SoundType type,
    float loudness, float radius, float volume,
    SOUND_LABEL playLabel)
{
    WorldSoundEvent ev{};
    ev.Position = pos;
    ev.Type = type;
    ev.Loudness = loudness;
    ev.Radius = radius;
    ev.Volume = volume;
    ev.PlayLabel = playLabel;

    EmitSound(ev);
}