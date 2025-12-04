#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include "system/commontypes.h"
#include "ParticleEmitter.h"

// 前方宣言
class GameObject;

class ParticleComponent : public IComponent
{
public:
    ParticleComponent();
    ~ParticleComponent() override = default;

    void Attach(EngineContext& context) override;
    void Detach() override;

    void Init(void) override;
    void Update(const float deltatime) override;
    void Uninit(void) override;

    ParticleEmitter& GetEmitter() { return m_Emitter; }
    const ParticleEmitter& GetEmitter() const { return m_Emitter; }

    // 必要に応じて外から設定できるようにする
    void SetLocalOffset(const DirectX::XMFLOAT3& offset) { m_LocalOffset = offset; }

private:
    EngineContext* m_Context = nullptr;
    ParticleEmitter m_Emitter;

    // Owner の原点からのオフセット（カメラの少し上など）
    Vector3 m_LocalOffset{ 0.0f, 0.0f, 0.0f };
};