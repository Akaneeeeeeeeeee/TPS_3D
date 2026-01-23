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

	m_Looping.fill(false);
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
#if _DEBUG
    //auto L = listenerPos;
    //printf("[Sound] listener=(%.2f,%.2f,%.2f)\n", L.x, L.y, L.z);
#endif
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

    // 念のためUIループも停止
    for (int i = 0; i < (int)SOUND_LABEL_MAX; ++i)
    {
        if (m_Looping[i])
        {
            m_System.StopLoop((SOUND_LABEL)i);
            m_Looping[i] = false;
        }
    }

    // 2) 再生中のOneShotを回収してから解放
    m_System.Update();
    m_System.Uninit();
}

void SoundHub::StartUILoop(SOUND_LABEL label, float volume01)
{
    if (!IsValid(label)) return;
    volume01 = std::clamp(volume01, 0.0f, 1.0f);

    if (!m_Looping[(int)label])
    {
        m_System.PlayLoop(label, volume01);     // 1回だけ開始
        m_Looping[(int)label] = true;
    }
    else
    {
        m_System.SetLoopVolume(label, volume01); // 以降は音量だけ
    }
}

void SoundHub::StopUILoop(SOUND_LABEL label)
{
    if (!IsValid(label)) return;

    if (m_Looping[(int)label])
    {
        m_System.StopLoop(label);
        m_Looping[(int)label] = false;
    }
}

void SoundHub::SetUILoopVolume(SOUND_LABEL label, float volume01)
{
    if (!IsValid(label)) return;
    if (!m_Looping[(int)label]) return;

    volume01 = std::clamp(volume01, 0.0f, 1.0f);
    m_System.SetLoopVolume(label, volume01);
}