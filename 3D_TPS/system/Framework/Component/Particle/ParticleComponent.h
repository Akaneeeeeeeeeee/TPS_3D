#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include "system/commontypes.h"
#include "ParticleEmitter.h"

// 前方宣言
class GameObject;
class WeatherSystem;

class ParticleComponent : public IComponent
{
public:
    ParticleComponent();
    ~ParticleComponent() override = default;

    void Attach(EngineServices& context) override;
    void Detach() override;

    void Init(void) override;
    void Update(const float deltatime) override;
    void Uninit(void) override;

    ParticleEmitter& GetEmitter() { return m_Emitter; }
    const ParticleEmitter& GetEmitter() const { return m_Emitter; }

    // 必要に応じて外から設定できるようにする
    void SetLocalOffset(const DirectX::XMFLOAT3& offset) { m_LocalOffset = offset; }
	// 生成範囲の設定（XZ 平面、中心は Owner の位置 + オフセット）
    void SetSpawnAreaXZ(float halfWidth, float halfDepth)
    {
        m_Emitter.SetSpawnAreaXZ(halfWidth, halfDepth);
	}

private:
    WeatherSystem* m_WeatherSystem;
    ParticleEmitter m_Emitter;

    // Owner の原点からのオフセット（カメラの少し上など）
    Vector3 m_LocalOffset{ 0.0f, 0.0f, 0.0f };
};