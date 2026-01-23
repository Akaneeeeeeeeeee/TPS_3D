#pragma once
#include <vector>
#include <algorithm>

#include "system/commontypes.h"
#include "system/Sound/WorldSoundEvent.h"
#include "Framework/Sound/IWorldSoundListener.h"

class SoundBus
{
public:
    void BeginFrame() { m_Events.clear(); }
    void Emit(const WorldSoundEvent& ev) { m_Events.push_back(ev); }

    void RegisterListener(IWorldSoundListener* l)
    {
        if (!l) return;
        if (std::find(m_Listeners.begin(), m_Listeners.end(), l) != m_Listeners.end()) return;
        m_Listeners.push_back(l);
    }
    void UnregisterListener(IWorldSoundListener* l)
    {
        if (!l) return;
        m_Listeners.erase(std::remove(m_Listeners.begin(), m_Listeners.end(), l), m_Listeners.end());
    }

    void Dispatch()
    {
        for (const auto& ev : m_Events)
            for (auto* l : m_Listeners)
                if (l) l->OnWorldSound(ev);
    }

    void SetListenerPos(const Vector3& p) { m_ListenerPos = p; }
    const Vector3& GetListenerPos() const { return m_ListenerPos; }

private:
    std::vector<WorldSoundEvent> m_Events;
    std::vector<IWorldSoundListener*> m_Listeners;
    Vector3 m_ListenerPos = Vector3::Zero;
};
