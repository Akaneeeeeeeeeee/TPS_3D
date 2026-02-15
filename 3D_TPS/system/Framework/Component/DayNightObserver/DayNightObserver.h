#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/EngineSystem/EngineSystem.h"

class DayNightObserverComponent final : public IComponent
    , public IDayNightListener
{
public:
	DECLARE_COMPONENT_TYPE(DayNightObserverComponent, IComponent)
    // StreetLight 等が受け取り先を設定
    void SetReceiver(IDayNightListener* r)
    {
        m_Receiver = r;
        // 現在値を即反映したいならここで1回通知
        if (m_Receiver) m_Receiver->OnDayNightChanged(m_IsNight);
    }

    bool IsNightCached() const { return m_IsNight; } // 「取得だけ」用途

    // ---- IComponent ----
    void Attach(EngineServices& ctx) override
    {
        // contextに触るのはここだけ
        m_Weather = &ctx.weather;
        if (m_Weather)
            m_Weather->RegisterDayNightListener(this); // Register直後に通知される実装でもOK
    }

    void Detach(void) override
    {
        if (m_Weather)
            m_Weather->UnregisterDayNightListener(this);
        m_Weather = nullptr;
    }

    void Init(void) override
    {
        // 特に何もしない
	}

    void Update(const float) override
    {
        // 特に何もしない
	}

    void Uninit(void) override
    {
        // 念のため（Detachが確実なら不要だが安全）
        Detach();
        m_Receiver = nullptr;
    }

    // ---- IDayNightListener ----
    void OnDayNightChanged(bool isNight) override
    {
        m_IsNight = isNight;
        if (m_Receiver)
            m_Receiver->OnDayNightChanged(isNight);
    }

private:
    WeatherSystem* m_Weather = nullptr;
    IDayNightListener* m_Receiver = nullptr;
    bool               m_IsNight = false;
};
