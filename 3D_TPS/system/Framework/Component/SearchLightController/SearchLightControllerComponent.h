#pragma once
#include <algorithm>
#include <cmath>

#include "commontypes.h"
#include "Framework/Component/IComponent/IComponent.h"

// 前方宣言
class SpotLightComponent;

class SearchLightControllerComponent final : public IComponent
{
public:
    DECLARE_COMPONENT_TYPE(SearchLightControllerComponent, IComponent)

    SearchLightControllerComponent() = default;

    // WatchTower::Awake で渡す（GetComponent依存を無くす）
    void SetSpot(SpotLightComponent* spot) { m_spot = spot; }

    void Init() override;
    void Update(const float dt) override;
    void Uninit() override {}

    void Attach(EngineServices&) override {}
    void Detach() override {}

    void SetPowered(bool on) { m_powered = on; }

public:
    // 調整値
    float sweepMinDeg = -80.0f;
    float sweepMaxDeg = 80.0f;
    float yawSpeedDeg = 40.0f;
    float pitchDeg = -10.0f; // 下向き（マイナスで下）

private:
    void UpdatePatrol(float dt);
    void ApplyToSpot();

    Vector3 MakeDirFromYawPitchDeg(float yawDeg, float pitchDeg) const;

private:
    SpotLightComponent* m_spot = nullptr;

    bool  m_powered = true;

    float m_baseYawDeg = 0.0f;  // 初期の塔の向きを基準にする
    float m_yawLocalDeg = 0.0f; // -80..80 の首振り（ローカル）
    float m_dirSign = 1.0f;     // +1 / -1
};