#include "ParticleComponent.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Component/Transform/Transform.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"
#include "system/Framework/EngineSystem/EngineSystem.h"

using namespace DirectX;

ParticleComponent::ParticleComponent()
	: IComponent(), m_WeatherSystem(nullptr)
{
}

void ParticleComponent::Attach(EngineServices& context)
{
    // ここで登録
    m_WeatherSystem = &context.weather;
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
    // デフォルト設定（雨を想定）
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
        const auto& mtx = owner->GetWorldMatrix();
        const Vector3 pos = mtx.Translation();

        XMFLOAT3 origin(
            pos.x + m_LocalOffset.x,
            pos.y + m_LocalOffset.y,
            pos.z + m_LocalOffset.z);

        m_Emitter.SetOrigin(origin);
    }

    // 粒子更新
    m_Emitter.Update(deltatime);
}

void ParticleComponent::Uninit(void)
{
    m_Emitter.Clear();
}
