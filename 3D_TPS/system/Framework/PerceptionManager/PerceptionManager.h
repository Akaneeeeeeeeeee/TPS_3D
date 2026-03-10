#pragma once
#include "Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/LightSystem/LightSystem.h"
#include "Framework/SoundManager/SoundManager.h"

struct PerceptionFactors
{
    float visibility = 1.0f; // 0..1
    float hearing = 1.0f; // 0..1
};

class PerceptionManager
{
public:
    void Attach(WeatherSystem* w, LightSystem* l, SoundManager* s)
    {
        m_Weather = w;
        m_Light = l;
        m_Sound = s;
    }

    void Update()
    {
        // 1) 環境係数（天候/時刻）
        m_Env = m_Weather->GetPerceptionFactors(); // visibility/hearing を返すようにする

        // 2) ここでは “最終値の作り方（合成）” を管理
        //    visibility はライトが当たるほど 1 に近づく、など
        //    ※ライトは「地点ごと」なので Query で計算するのが自然
    }

    // 地点ごとの最終視認性（AIが使う）
    float GetVisibilityAt(const Vector3& pos) const
    {
        const float base = m_Env.visibility;               // 天候/時刻
        const float light = m_Light->GetLightFactorAt(pos);// ライト
        // ライトに当たるほど 1.0 に近づける
        return std::clamp(base + (1.0f - base) * light, 0.0f, 1.0f);
    }

    float GetHearingFactor() const
    {
        return m_Env.hearing; // 天候由来だけならこれで十分
    }

    const std::vector<WorldSoundEvent>& GetSoundEvents() const
    {
        return m_Sound->GetEvents();
    }

private:
    WeatherSystem* m_Weather = nullptr;
    LightSystem* m_Light = nullptr;
    SoundEventSystem* m_Sound = nullptr;

    PerceptionFactors m_Env{};
};
