#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "system/commontypes.h"

class LightSystem;
struct SpotLightGPU;

/*
* @brief	スポットライトコンポーネントクラス
* @detail	スポットライトを表現するコンポーネント
* @auther	赤根 和樹
* @date		2025/12/21
*/
class SpotLightComponent final : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(SpotLightComponent, IComponent)
    enum class AimMode
    {
        OwnerForward, // OwnerのForward方向
        WorldDown,    // 常に(0,-1,0)
        BelowY        // (x, groundY, z) を狙う
    };

    SpotLightComponent() = default;

    void Init() override {}
    void Update(const float) override {}
    void Uninit() override {}

    void Attach(EngineServices& context) override;
    void Detach() override;

    // ---- パラメータ ----
    void SetEnabled(bool e) { m_Enabled = e; }
    void SetColor(const Color& c) { m_Color = c; }
    void SetRange(float r) { m_Range = std::max(r, 0.0f); }
    void SetIntensity(float i) { m_Intensity = std::max(i, 0.0f); }

    // 角度（度） inner <= outer
    void SetAnglesDeg(float innerDeg, float outerDeg);

    // 向け方
    void SetAimOwnerForward() { m_AimMode = AimMode::OwnerForward; }
    void SetAimWorldDown() { m_AimMode = AimMode::WorldDown; }
    void SetAimBelowY(float groundY) { m_AimMode = AimMode::BelowY; m_GroundY = groundY; }

    // LightSystem が呼ぶ：GPU用に詰める
    bool BuildGPU(SpotLightGPU& out) const;

    // LightSystem が呼ぶ：知覚用（0〜1）
    // “ライトの当たり判定”＝円錐内 + 距離減衰 + 角度減衰
    float ComputeInfluence01(const Vector3& worldPos) const;

    void SetTopRadius(float topRadius);
    void SetNear(float n);

    void FitToGroundCircle(float groundY, float groundRadius,
        float topRadiusMin,
        float innerRatio,
        float nearMinAxis,
        float rangeExtraAxis);

    // 遮蔽（壁で隠れる）をやるなら LightSystem 側で RayCast するのがおすすめ
    // → Component は “形状” と “設定値” に寄せる

private:
    Vector3 GetLightPos() const;
    Vector3 GetLightDir() const; // 正規化済み
private:
    LightSystem* m_System = nullptr;

	bool  m_Enabled = true;             // 有効化フラグ
	Color m_Color = Color(1, 1, 1, 1);  // 色
	float m_Range = 2000.0f;            // 距離
	float m_Intensity = 1.0f;           // 強さ
    float m_Near = 30.0f;               // 光が始まる距離（=円錐台の上面位置）


    // cosで持つ（シェーダで使いやすい）
    float m_InnerCos = 0.95f;
    float m_OuterCos = 0.85f;

    AimMode m_AimMode = AimMode::OwnerForward;
    float   m_GroundY = 0.0f;
};
