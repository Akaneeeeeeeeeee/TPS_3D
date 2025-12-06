#pragma once
#include "system/commontypes.h"
#include "system/RandomEngine.h"

/*
* @brief	パーティクル構造体
* @detail	パーティクルの粒子一つの情報を格納する構造体
* @remark	粒子の位置、速度、寿命な管理する
* @auther	赤根　和樹
* @date		2025/12/04
*/
struct ParticleInstance
{
    DirectX::XMFLOAT3 pos;      // 位置
    DirectX::XMFLOAT3 vel;      // 速度
    float             life;     // 残り寿命
    float             maxLife;  // 初期寿命
};

/*
* @brief    パーティクルエミッタークラス
* @detail   パーティクルの生成と更新、描画を担当するクラス
* @remark   エミッターの位置、生成レート、初速、寿命範囲などを設定できる
* @auther   赤根　和樹
* @date     2025/12/04
*/
class ParticleEmitter
{
public:
    explicit ParticleEmitter(const DirectX::XMFLOAT3& origin = { 0.0f, 0.0f, 0.0f });

    // 設定系
    void SetOrigin(const DirectX::XMFLOAT3& origin);
    void SetEmitRate(float particlesPerSec);          // 1秒あたり生成数
    void SetLifeRange(float minLife, float maxLife);  // 寿命範囲
    void SetSpeedRange(float minSpeed, float maxSpeed);
    void SetDirection(const DirectX::XMFLOAT3& dir);  // 基本の発射方向
    void SetGravity(const DirectX::XMFLOAT3& gravity);
    void SetMaxParticles(size_t maxCount);

    // 更新
    void Update(float dt);

    // 全破棄
    void Clear();

    // 描画用に粒配列へアクセス（GPU 転送などに使う）
    const std::vector<ParticleInstance>& GetParticles() const { return m_Particles; }

    // 発生範囲の設定（XZ 平面、中心は m_Origin）
    void SetSpawnAreaXZ(float halfWidth, float halfDepth)
    {
        m_SpawnHalfWidth = std::max(0.0f, halfWidth);
        m_SpawnHalfDepth = std::max(0.0f, halfDepth);
    }

    // 発生高さのオフセット（m_Origin.y からの相対）
    void SetSpawnHeight(float height)
    {
        m_SpawnHeight = height;
    }

private:
    void SpawnParticles(float dt);
    ParticleInstance CreateOneParticle();

    DirectX::XMFLOAT3 m_Origin{};
    DirectX::XMFLOAT3 m_Direction{};  // 正規化前提
    DirectX::XMFLOAT3 m_Gravity{ 0.0f, -9.8f, 0.0f };

    float m_EmitRate = 0.0f;   // 1秒あたり生成数
    float m_EmitAcc = 0.0f;   // 累積値
    float m_MinLife = 0.5f;
    float m_MaxLife = 1.0f;
    float m_MinSpeed = 1.0f;
    float m_MaxSpeed = 1.0f;
    size_t m_MaxParticles = 1000;
    // 発生範囲
    float m_SpawnHalfWidth = 0.0f;  // X
    float m_SpawnHalfDepth = 0.0f;  // Z
    float m_SpawnHeight = 0.0f;  // Y

    std::vector<ParticleInstance> m_Particles;

    // 乱数エンジン
    RandomEngine m_Rng;
};