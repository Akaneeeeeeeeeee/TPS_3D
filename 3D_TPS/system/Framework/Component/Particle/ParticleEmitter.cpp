// ParticleEmitter.cpp
#include "ParticleEmitter.h"
#include <algorithm>

using namespace DirectX;

ParticleEmitter::ParticleEmitter(const XMFLOAT3& origin)
    : m_Origin(origin)
{
    // デフォルトコンストラクタで m_Rng はエントロピーから初期化される
    // 決定論にしたければ Reseed() を外から呼ぶ
}

void ParticleEmitter::SetOrigin(const XMFLOAT3& origin)
{
    m_Origin = origin;
}

void ParticleEmitter::SetEmitRate(float particlesPerSec)
{
    m_EmitRate = std::max(0.0f, particlesPerSec);
}

void ParticleEmitter::SetLifeRange(float minLife, float maxLife)
{
    m_MinLife = std::max(0.0f, minLife);
    m_MaxLife = std::max(m_MinLife, maxLife);
}

void ParticleEmitter::SetSpeedRange(float minSpeed, float maxSpeed)
{
    m_MinSpeed = std::max(0.0f, minSpeed);
    m_MaxSpeed = std::max(m_MinSpeed, maxSpeed);
}

void ParticleEmitter::SetDirection(const XMFLOAT3& dir)
{
    m_Direction = dir;

    XMVECTOR v = XMLoadFloat3(&m_Direction);
    if (!XMVector3Equal(v, XMVectorZero()))
    {
        v = XMVector3Normalize(v);
        XMStoreFloat3(&m_Direction, v);
    }
    else
    {
        m_Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
    }
}

void ParticleEmitter::SetGravity(const XMFLOAT3& gravity)
{
    m_Gravity = gravity;
}

void ParticleEmitter::SetMaxParticles(size_t maxCount)
{
    m_MaxParticles = maxCount;
    m_Particles.reserve(m_MaxParticles);
}

void ParticleEmitter::Clear()
{
    m_Particles.clear();
    m_EmitAcc = 0.0f;
}

void ParticleEmitter::Update(float dt)
{
    // 既存パーティクル更新
    for (size_t i = 0; i < m_Particles.size(); )
    {
        auto& p = m_Particles[i];

        p.life -= dt;
        if (p.life <= 0.0f)
        {
            p = m_Particles.back();
            m_Particles.pop_back();
            continue;
        }

        // 重力
        p.vel.x += m_Gravity.x * dt;
        p.vel.y += m_Gravity.y * dt;
        p.vel.z += m_Gravity.z * dt;

        // 位置
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.pos.z += p.vel.z * dt;

        ++i;
    }

    // 新規生成
    SpawnParticles(dt);
}

void ParticleEmitter::SpawnParticles(float dt)
{
    if (m_EmitRate <= 0.0f || m_MaxParticles == 0)
        return;

    m_EmitAcc += m_EmitRate * dt;

    int spawnCount = static_cast<int>(m_EmitAcc);
    if (spawnCount <= 0)
        return;

    m_EmitAcc -= static_cast<float>(spawnCount);

    for (int i = 0; i < spawnCount; ++i)
    {
        if (m_Particles.size() >= m_MaxParticles)
            break;

        m_Particles.push_back(CreateOneParticle());
    }
}

ParticleInstance ParticleEmitter::CreateOneParticle()
{
    ParticleInstance p{};

    // 寿命
    double life = m_Rng.uniformReal(static_cast<double>(m_MinLife),
        static_cast<double>(m_MaxLife));
    p.maxLife = static_cast<float>(life);
    p.life = p.maxLife;

    // 速度の大きさ
    double speed = m_Rng.uniformReal(static_cast<double>(m_MinSpeed),
        static_cast<double>(m_MaxSpeed));

    // 基本方向
    XMVECTOR dir = XMLoadFloat3(&m_Direction);
    if (XMVector3Equal(dir, XMVectorZero()))
    {
        dir = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
    }
    dir = XMVector3Normalize(dir);

    // 方向に少しばらつき
    double jx = m_Rng.uniformReal(-0.1, 0.1);
    double jy = m_Rng.uniformReal(-0.1, 0.1);
    double jz = m_Rng.uniformReal(-0.1, 0.1);

    dir = XMVectorAdd(
        dir,
        XMVectorSet(
            static_cast<float>(jx),
            static_cast<float>(jy),
            static_cast<float>(jz),
            0.0f));
    dir = XMVector3Normalize(dir);

    XMFLOAT3 v;
    XMStoreFloat3(&v, dir);

    p.vel.x = v.x * static_cast<float>(speed);
    p.vel.y = v.y * static_cast<float>(speed);
    p.vel.z = v.z * static_cast<float>(speed);

    // ---------------------------
    // 発生位置の決定
    // ---------------------------
    float ox = 0.0f;
    float oz = 0.0f;
    float oy = 0.0f;

    // XZ は -half ～ +half の一様乱数
    if (m_SpawnHalfWidth > 0.0f || m_SpawnHalfDepth > 0.0f)
    {
        ox = static_cast<float>(
            m_Rng.uniformReal(
                -static_cast<double>(m_SpawnHalfWidth),
                static_cast<double>(m_SpawnHalfWidth)));

        oz = static_cast<float>(
            m_Rng.uniformReal(
                -static_cast<double>(m_SpawnHalfDepth),
                static_cast<double>(m_SpawnHalfDepth)));
    }

    // Y は「原点を中心とした帯」の中で一様乱数
    //
    //   ・m_SpawnHeight == 0 のとき → ちょうど原点の高さだけ
    //   ・m_SpawnHeight > 0 のとき → [ -m_SpawnHeight, +m_SpawnHeight ] の範囲
    //
    if (m_SpawnHeight > 0.0f)
    {
        double yLocal = m_Rng.uniformReal(
            -static_cast<double>(m_SpawnHeight),
            static_cast<double>(m_SpawnHeight));
        oy = static_cast<float>(yLocal);
    }
    else
    {
        oy = 0.0f; // 原点ちょうど
    }

    p.pos.x = m_Origin.x + ox;
    p.pos.y = m_Origin.y + oy;
    p.pos.z = m_Origin.z + oz;

    return p;
}
