#pragma once
#include "system/Framework/NonCopyable/Singleton_Template.h"
#include "system/Sound/WorldSoundEvent.h"
#include <vector>
#include "system/Sound/SoundWaveVisualizer.h"

class SoundManager : public Singleton<SoundManager>
{
public:
    void BeginFrame()
    {
        m_Buffer.events.clear();
    }

    void EmitSound(const WorldSoundEvent& ev)
    {
        // 1) 知覚用に積む
        m_Buffer.events.push_back(ev);

        // 2) 出力/演出
        m_Output.OnEmit(ev);
    }

    const std::vector<WorldSoundEvent>& GetEvents() const
    {
        return m_Buffer.events;
    }

private:
    struct Buffer
    {
        std::vector<WorldSoundEvent> events;
    } m_Buffer;

    struct Output
    {
        void OnEmit(const WorldSoundEvent& ev)
        {
            // AudioSystem::Get().Play3DSound(...);  // 必要なら
            SoundWaveVisualizer::GetInstance().OnEmit(ev);
        }
    } m_Output;
};
