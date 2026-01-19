#pragma once

#include <vector>
#include <algorithm>

#include "system/commontypes.h"
#include "system/Sound/WorldSoundEvent.h"

class SoundEmitterComponent;
class WeatherSystem;
class SoundSystem;
class SoundWaveVisualizer;
class IWorldSoundListener;

class SoundManager
{
public:
    void BeginFrame() { m_Events.clear(); }
    void EmitSound(const WorldSoundEvent& ev) { m_Events.push_back(ev); }

    void RegisterListener(IWorldSoundListener* l);
    void UnregisterListener(IWorldSoundListener* l);

    // Scene/GameFeatures 更新後に 1回呼ぶ（このフレーム分を配布）
    void Dispatch();

    void SetListenerPos(const Vector3& p) { m_ListenerPos = p; }
    const Vector3& GetListenerPos() const { return m_ListenerPos; }

    // デバッグ用途で残してOK（Pullはしない）
    const std::vector<WorldSoundEvent>& GetEvents() const { return m_Events; }

    // 任意（Emitter登録が欲しければ）
    void RegisterEmitter(SoundEmitterComponent* e);
    void UnregisterEmitter(SoundEmitterComponent* e);

private:
    std::vector<WorldSoundEvent> m_Events;
    std::vector<IWorldSoundListener*> m_Listeners;

    std::vector<SoundEmitterComponent*> m_Emitters; // 任意
    Vector3 m_ListenerPos = Vector3::Zero;
};