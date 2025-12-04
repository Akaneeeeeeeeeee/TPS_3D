#include "WeatherSystem.h"

void WeatherSystem::SetWeather(WeatherType type, float transitionSec)
{
	m_SrcParams = m_CurrentParams;         // 今の状態を始点に
	m_DstParams = MakePreset(type);        // 目標プリセット
	m_CurrentWeather = type;

	m_TransitionTime = std::max(transitionSec, 0.0001f);
	m_TransitionT = 0.0f;
}