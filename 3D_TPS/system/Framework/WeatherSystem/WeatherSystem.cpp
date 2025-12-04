#include "WeatherSystem.h"
#include "system/Framework/Component/Particle/ParticleComponent.h"
#include <cassert>
#include <DirectXMath.h>

using DirectX::XMFLOAT3;

WeatherSystem::WeatherSystem()
{
    // 初期状態は Clear のプリセットにしておく
    m_CurrentParams = MakePreset(WeatherType::Clear);
    m_SrcParams = m_CurrentParams;
    m_DstParams = m_CurrentParams;
    m_TransitionTime = 1.0f;
    m_TransitionT = 1.0f;
}

void WeatherSystem::Register(ParticleComponent* comp)
{
    if (!comp) return;

    auto it = std::find(m_ParticleComponents.begin(),
        m_ParticleComponents.end(),
        comp);
    if (it == m_ParticleComponents.end())
    {
        m_ParticleComponents.push_back(comp);
    }
}

void WeatherSystem::Unregister(ParticleComponent* comp)
{
    if (!comp) return;

    auto it = std::remove(m_ParticleComponents.begin(),
        m_ParticleComponents.end(),
        comp);
    if (it != m_ParticleComponents.end())
    {
        m_ParticleComponents.erase(it, m_ParticleComponents.end());
    }
}


void WeatherSystem::Update(float dt)
{
    // パラメータの遷移
    if (m_TransitionT < 1.0f)
    {
        m_TransitionT += dt / m_TransitionTime;
        if (m_TransitionT > 1.0f) m_TransitionT = 1.0f;

        m_CurrentParams = LerpParams(m_SrcParams, m_DstParams, m_TransitionT);
    }

    ApplyToParticles();
    // 今後 Fog / Sky もここで反映してよい
}

void WeatherSystem::SetWeather(WeatherType type, float transitionSec)
{
    m_SrcParams = m_CurrentParams;      // 今の状態を始点に
    m_DstParams = MakePreset(type);     // 目標プリセット
    m_CurrentWeather = type;

    m_TransitionTime = std::max(transitionSec, 0.0001f);
    m_TransitionT = 0.0f;
}

// パラメータ補間
WeatherParticleParams LerpParams(const WeatherParticleParams& a, const WeatherParticleParams& b, float t)
{
    auto lerp = [](float x, float y, float t) {
        return x + (y - x) * t;
        };

    WeatherParticleParams r{};

    r.rainEmitRate = lerp(a.rainEmitRate, b.rainEmitRate, t);
    r.rainMinLife = lerp(a.rainMinLife, b.rainMinLife, t);
    r.rainMaxLife = lerp(a.rainMaxLife, b.rainMaxLife, t);
    r.rainMinSpeed = lerp(a.rainMinSpeed, b.rainMinSpeed, t);
    r.rainMaxSpeed = lerp(a.rainMaxSpeed, b.rainMaxSpeed, t);

    r.sandEmitRate = lerp(a.sandEmitRate, b.sandEmitRate, t);
    r.sandMinLife = lerp(a.sandMinLife, b.sandMinLife, t);
    r.sandMaxLife = lerp(a.sandMaxLife, b.sandMaxLife, t);
    r.sandMinSpeed = lerp(a.sandMinSpeed, b.sandMinSpeed, t);
    r.sandMaxSpeed = lerp(a.sandMaxSpeed, b.sandMaxSpeed, t);

    r.rainDir = a.rainDir;   // 向きはとりあえず切り替えだけ
    r.sandDir = a.sandDir;

    r.fogDensity = lerp(a.fogDensity, b.fogDensity, t);
    r.fogColor.x = lerp(a.fogColor.x, b.fogColor.x, t);
    r.fogColor.y = lerp(a.fogColor.y, b.fogColor.y, t);
    r.fogColor.z = lerp(a.fogColor.z, b.fogColor.z, t);

    return r;
}


void WeatherSystem::ApplyToParticles()
{
    // 1) 現在の天候から有効な粒パラメータを決める
    float   emitRate = 0.0f;
    float   minLife = 0.0f;
    float   maxLife = 0.0f;
    float   minSpeed = 0.0f;
    float   maxSpeed = 0.0f;
    XMFLOAT3 dir = { 0.0f, -1.0f, 0.0f };
    XMFLOAT3 gravity = { 0.0f, -9.8f, 0.0f };

    // 雨が有効（小雨・土砂降り）
    if (m_CurrentParams.rainEmitRate > 0.0f)
    {
        emitRate = m_CurrentParams.rainEmitRate;
        minLife = m_CurrentParams.rainMinLife;
        maxLife = m_CurrentParams.rainMaxLife;
        minSpeed = m_CurrentParams.rainMinSpeed;
        maxSpeed = m_CurrentParams.rainMaxSpeed;
        dir = m_CurrentParams.rainDir;
        gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);
    }
    // 砂嵐が有効
    else if (m_CurrentParams.sandEmitRate > 0.0f)
    {
        emitRate = m_CurrentParams.sandEmitRate;
        minLife = m_CurrentParams.sandMinLife;
        maxLife = m_CurrentParams.sandMaxLife;
        minSpeed = m_CurrentParams.sandMinSpeed;
        maxSpeed = m_CurrentParams.sandMaxSpeed;
        dir = m_CurrentParams.sandDir;
        gravity = XMFLOAT3(0.0f, -1.0f, 0.0f);   // 少し舞う感じなら弱め
    }
    // Clear はどちらも 0 → emitRate = 0 のまま

    // 2) 登録済み ParticleComponent 全員に反映
    for (auto* comp : m_ParticleComponents)
    {
        if (!comp) continue;

        auto& emitter = comp->GetEmitter();

        emitter.SetEmitRate(emitRate);
        emitter.SetLifeRange(minLife, maxLife);
        emitter.SetSpeedRange(minSpeed, maxSpeed);
        emitter.SetDirection(dir);
        emitter.SetGravity(gravity);
    }
}
