#include "ParticleComponent.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Component/Transform/Transform.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"

using namespace DirectX;

ParticleComponent::ParticleComponent()
    : IComponent()
{
}

void ParticleComponent::Attach(EngineContext& context)
{
    // ここで登録
    m_WeatherSystem = &context.weatherSystem;
    if (m_WeatherSystem)
    {
        m_WeatherSystem->Register(this);
    }
}

void ParticleComponent::Detach(void)
{
    // 登録解除
    if (m_WeatherSystem)
    { 
        m_WeatherSystem->Unregister(this);
        m_WeatherSystem = nullptr;
    }
}

void ParticleComponent::Init(void)
{
    // デフォルト設定（例として雨を想定）
    m_Emitter.SetEmitRate(0.0f);          // 最初は出さない
    m_Emitter.SetLifeRange(0.5f, 1.0f);
    m_Emitter.SetSpeedRange(10.0f, 20.0f);
    m_Emitter.SetDirection(XMFLOAT3(0.0f, -1.0f, 0.0f));
    m_Emitter.SetGravity(XMFLOAT3(0.0f, -9.8f, 0.0f));
    m_Emitter.SetMaxParticles(2000);
}

void ParticleComponent::Update(const float deltatime)
{
    if (!GetIsValid())
        return;

    // Owner の位置に追従
    if (GameObject* owner = GetOwner())
    {
        // Transform の取得方法は実装に合わせて変えてください
        // 例:
        // const Vector3 pos = owner->GetTransform().GetWorldPosition();
        // ここでは XMFLOAT3 として仮に変換
        const auto& t = owner->GetTransform();
        const auto  wp = t.GetPosition();  // Vector3 を返すと仮定

        XMFLOAT3 origin(
            wp.x + m_LocalOffset.x,
            wp.y + m_LocalOffset.y,
            wp.z + m_LocalOffset.z);

        m_Emitter.SetOrigin(origin);
    }

    // 粒子更新
    m_Emitter.Update(deltatime);
}

void ParticleComponent::Uninit(void)
{
    m_Emitter.Clear();
}
