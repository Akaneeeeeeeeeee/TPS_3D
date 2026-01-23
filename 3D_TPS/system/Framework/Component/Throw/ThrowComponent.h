#pragma once
#include <array>
#include <memory>
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "IThrowAction.h"
#include "IThrowEventListener.h"

class ThrowComponent final : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(ThrowComponent, IComponent)
    void Init() override;
    void Update(float dt) override {};
    void LateUpdate(float dt) override;
    void Uninit() override;

    // Player/Enemy からの通知
    void OnAimStart();
    void OnAimEnd();

    // 投げ入力（押された瞬間）
    void Throw(ThrowItemId id);

	void Attach(EngineServices& ctx) override {}
	void Detach() override {}
    void SetThrowEventListener(IThrowEventListener* i) { m_Listener = i; }

private:
    enum class State { None, Hold, Throwing };

    void TryResolveAnim();
    void EnsureActionsBuilt();
    IThrowAction* FindAction(ThrowItemId id);

    void EnterHold();
    void ApplyHoldEveryFrame();
    float SanitizeReleaseNorm(const ThrowTuning& t) const;

    Vector3 GetAimForward() const;
    Vector3 ComputeThrowVelocity(const ThrowTuning& t) const;
    void DrawThrowGuide(const Vector3& startPos, const Vector3& startVel) const;

private:
    SkinnedAnimationComponent* m_anim = nullptr;

    std::array<std::unique_ptr<IThrowAction>, 8> m_actions{};
    bool m_actionsBuilt = false;

    IThrowAction* m_current = nullptr;

    bool  m_isAiming = false;
    State m_state = State::None;
    ThrowItemId m_previewId = ThrowItemId::Rock; // 構え中ガイド表示用（最後に選んだ投げ物）

	IThrowEventListener* m_Listener = nullptr;
    bool  m_hasSpawned = false;
    float m_elapsed = 0.0f;
    float m_cooldown = 0.0f;
};
