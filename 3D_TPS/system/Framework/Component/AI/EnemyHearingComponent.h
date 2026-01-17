#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/SoundManager/SoundManager.h"

// 前方宣言
class EnemyAIComponent;
class PhysicsManager;
class WeatherSystem;

/*
* @brief    敵聴覚コンポーネント
* @detail   周囲で発生した音を検知し、最も大きな音の発生源を敵AIに通知するコンポーネント
* @author   赤根　和樹
* @date     2025/11/20
*/
class EnemyHearingComponent : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(EnemyHearingComponent, IComponent)
    EnemyHearingComponent() = default;
    ~EnemyHearingComponent() noexcept override = default;

    void Init(void) override {}
    void Update(const float deltaTime) override;
    void Uninit(void) override {}

    void Attach(EngineServices& context) override;
    void Detach(void) override;

    void SetEnemyAI(EnemyAIComponent* ai);
    void SetBaseHearingRadius(float r) { m_BaseHearingRadius = r; }
    void SetMinScore(float s) { m_MinScore = s; }
    // 耳の高さ（敵のローカル Y オフセット）
    void SetEarHeight(float h) { m_EarHeight = h; }

    // SoundManager から呼ぶ
    void OnWorldSound(const WorldSoundEvent& ev);

private:
    // 距離減衰＋遮蔽係数を含めた「最終音量」を返す
    float ComputePerceivedLoudness(const WorldSoundEvent& ev) const;

    // 晴天時の基本聴覚半径
    float m_BaseHearingRadius = 20.0f;

    // 反応するスコアの下限（ノイズカット用。最初は 0 でよい）
    float m_MinScore = 0.0f;

    // だいたい頭くらいの高さ
    float m_EarHeight = 80.0f;

    // 閾値(これ未満は「聞こえない」扱い)
    // どのあたりから「もう聞こえない扱い」にするか調整できる
    float m_Threshold = 0.1f;       

	EnemyAIComponent* m_pEnemyAI = nullptr;
	PhysicsManager* m_pPhysics = nullptr;
	WeatherSystem* m_pWeather = nullptr;
};