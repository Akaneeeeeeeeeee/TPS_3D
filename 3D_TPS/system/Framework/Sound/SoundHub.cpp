#include "Framework/Sound/SoundHub.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"

void SoundHub::Init(WeatherSystem* weather)
{
    m_pWeather = weather;

    m_System.Init();

    m_Playback.SetBus(&m_Bus);
    m_Playback.SetWeatherSystem(m_pWeather);
    m_Playback.SetSoundSystem(&m_System);

    m_WeatherAudio.SetWeatherSystem(m_pWeather);
    m_WeatherAudio.SetSoundSystem(&m_System);

    m_Viz.SetWeatherSystem(m_pWeather);

    // リスナー登録（再生と可視化は内部で登録してしまう）
    m_Bus.RegisterListener(&m_Playback);
    m_Bus.RegisterListener(&m_Viz);
}

void SoundHub::BeginFrame(float dt)
{
    m_LastDt = dt;

    // 前フレームのOneShot回収
    m_System.Update();

    // イベントクリア
    m_Bus.BeginFrame();
}

void SoundHub::UpdateFrame(float dt, const Vector3& listenerPos)
{
    m_Bus.SetListenerPos(listenerPos);
    auto L = listenerPos;
    printf("[Sound] listener=(%.2f,%.2f,%.2f)\n", L.x, L.y, L.z);
    // 天候ループ音（位置なし）
    m_WeatherAudio.Update(dt);
}

void SoundHub::EndFrame(float dt)
{
    // このフレームのイベントを配布（再生/可視化/EnemyHearingなど）
    m_Bus.Dispatch();

    // 可視化の寿命更新
    m_Viz.Update(dt);
}

void SoundHub::Uninit()
{
    // 1) 天候ループ音を止める（スロットが2本なら両方止める）
    m_WeatherAudio.Uninit();

    // 2) 再生中のOneShotを回収してから解放
    m_System.Update();
    m_System.Uninit();
}