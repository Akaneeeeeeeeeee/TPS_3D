#pragma once
#include "Framework/GameObject/GameObject.h"
#include "Framework/Component/Light/SpotLightComponent.h"

// 前方宣言
class DayNightObserverComponent;

/*
* @brief    街灯ゲームオブジェクト
* @detail   スポットライトコンポーネントを持つ街灯用オブジェクト
* @auther   赤根 和樹
* @date     2025/12/21
*/
class StreetLight final : public GameObject, public IDayNightListener
{
public:
    DECLARE_GAMEOBJECT_TYPE(StreetLight, GameObject)
    // ObjectManager/ComponentFactory の作りに合わせてコンストラクタは GameObject と同形にする
    StreetLight(ComponentFactory* factory,
        const uint64_t id,
        const std::string& name = "",
        const Tag tag = Tag::Light,
        const Transform& transform = Transform::One())
        : GameObject(factory, id, name, tag, transform)
    {
    }

    // GameObjectライフサイクル
    void Awake() override;
    void Start() override;
    void Uninit() override;

    void OnDayNightChanged(bool isNight) override;

    // ---- 触りたい代表パラメータ（プレハブの外から変えられる）----
    void SetNightOnly(bool e);
    void SetEnabled(bool e);
    void SetColor(const Color& c);
    void SetIntensity(float i);

    void SetGroundCircle(float groundRadius, float groundY, float topRadiusMin, float innerRatio);

private:
    // 保留パラメータ群
    struct Pending
    {
        bool  enabled = true;
        // 夜間のみ点灯
        bool  nightOnly = true;

        Color color = Color(1, 0.95f, 0.8f, 1);
        float intensity = 6.0f;

        // 自動フィット
        bool  useGroundFit = false;
        float groundRadius = 500.0f;
        float groundY = 0.0f;
        float topRadiusMin = 50.0f;
        float innerRatio = 0.6f;

        SpotLightComponent::AimMode aimMode = SpotLightComponent::AimMode::WorldDown;
    } m_Pending;

private:
    // 生成済みなら直接触る（未生成の間は pending に溜める）
    SpotLightComponent* m_Spot = nullptr;
    DayNightObserverComponent* m_DayNight = nullptr;

    bool m_IsNight = false;
    bool m_RuntimeLit = true; // 前回の最終状態

    void ApplyPendingToSpot();
    void RefreshLighting();
};
