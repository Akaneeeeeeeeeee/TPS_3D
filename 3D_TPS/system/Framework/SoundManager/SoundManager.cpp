#include "Framework/SoundManager/SoundManager.h"

#include "system/Framework/SoundManager/SoundSystem.h"
#include "system/Sound/SoundWaveVisualizer.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"

void SoundManager::RegisterListener(IWorldSoundListener* l)
{
    if (!l) return;
    if (std::find(m_Listeners.begin(), m_Listeners.end(), l) != m_Listeners.end()) return;
    m_Listeners.push_back(l);
}

void SoundManager::UnregisterListener(IWorldSoundListener* l)
{
    if (!l) return;
    m_Listeners.erase(std::remove(m_Listeners.begin(), m_Listeners.end(), l), m_Listeners.end());
}

void SoundManager::Dispatch()
{
    for (const auto& ev : m_Events)
    {
        for (auto* l : m_Listeners)
        {
            if (l) l->OnWorldSound(ev);
        }
    }
}

// ”CˆÓi‚È‚­‚Ä‚à¬—§j
void SoundManager::RegisterEmitter(SoundEmitterComponent* e)
{
    if (!e) return;
    if (std::find(m_Emitters.begin(), m_Emitters.end(), e) != m_Emitters.end()) return;
    m_Emitters.push_back(e);
}
void SoundManager::UnregisterEmitter(SoundEmitterComponent* e)
{
    if (!e) return;
    m_Emitters.erase(std::remove(m_Emitters.begin(), m_Emitters.end(), e), m_Emitters.end());
}