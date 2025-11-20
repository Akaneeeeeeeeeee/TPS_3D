#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/SoundManager/SoundManager.h"

// 前方宣言
class EnemyAIComponent;

/*
* @brief    敵聴覚コンポーネント
* @detail   周囲で発生した音を検知し、最も大きな音の発生源を敵AIに通知するコンポーネント
* @author   赤根　和樹
* @date     2025/11/20
*/
class EnemyHearingComponent : public IComponent
{
public:
    EnemyHearingComponent() = default;
    ~EnemyHearingComponent() noexcept override = default;

    void Init(void) override {}
    void Update(const float deltaTime) override;
    void Uninit(void) override {}

    void Attach(EngineContext& context) override {}
    void Detach(void) override {}

    void SetEnemyAI(EnemyAIComponent* ai);
    void SetBaseHearingRadius(float r) { m_BaseHearingRadius = r; }
    void SetMinScore(float s) { m_MinScore = s; }

private:
    // 晴天時の基本聴覚半径
    float m_BaseHearingRadius = 20.0f;

    // 反応するスコアの下限（ノイズカット用。最初は 0 でよい）
    float m_MinScore = 0.0f;

	EnemyAIComponent* m_pEnemyAI = nullptr;
};