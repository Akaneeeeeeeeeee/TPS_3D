#pragma once
#include "commontypes.h"
#include "Framework/NonCopyable/Singleton_Template.h"

/*
* @brief	Timeクラス
* @detail	ゲーム内の時間を管理するクラス
* @remark	時間の更新、デルタタイム、トータルタイムの取得が可能
* @auther	赤根和樹
* @date		2025/11/08
*/
class Time : public Singleton<Time>
{
    friend class Singleton<Time>;
public:
    Time();
    ~Time();

    void Update(const uint64_t deltaMicroSec)
    {
        // μ秒 → 秒 に変換
        using sec = std::chrono::duration<float>;
        m_UnscaledDeltaTime = sec(std::chrono::microseconds(deltaMicroSec)).count();

        m_DeltaTime = m_UnscaledDeltaTime * m_TimeScale;

        m_UnscaledTotalTime += m_UnscaledDeltaTime;
        m_TotalTime += m_DeltaTime;
    }

    float Deltatime(void) const { return m_DeltaTime; }
    float UnscaledDeltatime(void) const { return m_UnscaledDeltaTime; }

    float Totaltime(void) const { return m_TotalTime; }
    float UnscaledTotaltime(void) const { return m_UnscaledTotalTime; }

    void SetTimeScale(float scale) {
        m_TimeScale = std::max(scale, 0.0f);
    }
private:
    float m_TimeScale = 1.0f;

    float m_DeltaTime = 0.0f;
    float m_UnscaledDeltaTime = 0.0f;

    float m_TotalTime = 0.0f;
    float m_UnscaledTotalTime = 0.0f;
};

Time::Time()
{
}

Time::~Time()
{
}