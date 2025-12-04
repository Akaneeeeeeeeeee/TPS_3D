#pragma once

// 天候タイプ
enum class WeatherType {
    Clear,          // 快晴
    LightRain,      // 小雨
    HeavyRain,      // 土砂降り
    Sandstorm,      // 砂嵐
    // Snow, Fog …を追加してもよい

    Weather_MAX
};

// 天候パラメータ
struct WeatherParticleParams
{
    // 雨用
    float           rainEmitRate = 0.0f;    // 1秒あたり生成数
    float           rainMinLife = 0.0f;
    float           rainMaxLife = 0.0f;
    float           rainMinSpeed = 0.0f;
    float           rainMaxSpeed = 0.0f;
    DirectX::XMFLOAT3 rainDir = { 0.0f, -1.0f, 0.0f };

    // 砂嵐用
    float           sandEmitRate = 0.0f;
    float           sandMinLife = 0.0f;
    float           sandMaxLife = 0.0f;
    float           sandMinSpeed = 0.0f;
    float           sandMaxSpeed = 0.0f;
    DirectX::XMFLOAT3 sandDir = { 1.0f, 0.0f, 0.0f };

    // ここに霧や空のパラメータも足せる
    float           fogDensity = 0.0f;
    DirectX::XMFLOAT3 fogColor = { 1.0f, 1.0f, 1.0f };
};

/*
* @brief    天候プリセットを取得
* @detail   指定した天候タイプに応じたパラメータを返す
*/
inline WeatherParticleParams MakePreset(WeatherType type)
{
    WeatherParticleParams p{};

    using namespace DirectX;

    switch (type)
    {
    case WeatherType::Clear:
        // 何も出さない
        p.rainEmitRate = 0.0f;
        p.sandEmitRate = 0.0f;
        p.fogDensity = 0.0f;
        break;

    case WeatherType::LightRain:
        p.rainEmitRate = 200.0f;
        p.rainMinLife = 0.7f;
        p.rainMaxLife = 1.2f;
        p.rainMinSpeed = 40.0f;
        p.rainMaxSpeed = 60.0f;
        p.rainDir = XMFLOAT3(0.0f, -1.0f, 0.0f);

        p.sandEmitRate = 0.0f;
        p.fogDensity = 0.002f;
        p.fogColor = XMFLOAT3(0.7f, 0.7f, 0.8f);
        break;

    case WeatherType::HeavyRain:
        p.rainEmitRate = 800.0f;
        p.rainMinLife = 0.8f;
        p.rainMaxLife = 1.4f;
        p.rainMinSpeed = 80.0f;
        p.rainMaxSpeed = 120.0f;
        p.rainDir = XMFLOAT3(0.0f, -1.0f, 0.0f);

        p.sandEmitRate = 0.0f;
        p.fogDensity = 0.006f;
        p.fogColor = XMFLOAT3(0.6f, 0.6f, 0.7f);
        break;

    case WeatherType::Sandstorm:
        p.rainEmitRate = 0.0f;

        p.sandEmitRate = 600.0f;
        p.sandMinLife = 3.0f;
        p.sandMaxLife = 5.0f;
        p.sandMinSpeed = 20.0f;
        p.sandMaxSpeed = 40.0f;
        p.sandDir = XMFLOAT3(1.0f, 0.2f, 0.0f); // 斜め方向に飛ばす

        p.fogDensity = 0.01f;
        p.fogColor = XMFLOAT3(0.8f, 0.7f, 0.4f);
        break;
    }

    return p;
}

// 前方宣言
class ParticleComponent;

/*
* @brief    WeatherSystemクラス
* @detail   天候の変化を管理するシステム
* @remark   このクラスが次の天候の決定、天候の遷移を担当する。
* @auther   赤根　和樹
* @date     2025/12/04
*/
class WeatherSystem
{
public:
    WeatherSystem() = default;

    // ParticleComponent の登録 / 解除
    void Register(ParticleComponent* comp);
    void Unregister(ParticleComponent* comp);

    // 天候指定（t秒かけて遷移）
    void SetWeather(WeatherType type, float transitionSec);

    WeatherType GetWeather() const { return m_CurrentWeather; }

    // 毎フレーム呼ぶ
    void Update(float dt)
    {
        if (m_TransitionT < 1.0f)
        {
            m_TransitionT += dt / m_TransitionTime;
            if (m_TransitionT > 1.0f) m_TransitionT = 1.0f;

            m_CurrentParams = LerpParams(m_SrcParams, m_DstParams, m_TransitionT);
        }

        ApplyToParticles();
        // ApplyToSky();
        // ApplyToFog();
    }

private:
    static WeatherParticleParams LerpParams(
        const WeatherParticleParams& a,
        const WeatherParticleParams& b,
        float t)
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

        r.rainDir = a.rainDir;   // 方向はとりあえずスイッチだけ
        r.sandDir = a.sandDir;   // 必要なら正規化付き補間にしてもよい

        r.fogDensity = lerp(a.fogDensity, b.fogDensity, t);
        r.fogColor.x = lerp(a.fogColor.x, b.fogColor.x, t);
        r.fogColor.y = lerp(a.fogColor.y, b.fogColor.y, t);
        r.fogColor.z = lerp(a.fogColor.z, b.fogColor.z, t);

        return r;
    }

    void ApplyToParticles()
    {
        // 雨パーティクル
        if (m_RainComponent)
        {
            auto& emitter = m_RainComponent->GetEmitter();

            emitter.SetEmitRate(m_CurrentParams.rainEmitRate);
            emitter.SetLifeRange(m_CurrentParams.rainMinLife,
                m_CurrentParams.rainMaxLife);
            emitter.SetSpeedRange(m_CurrentParams.rainMinSpeed,
                m_CurrentParams.rainMaxSpeed);
            emitter.SetDirection(m_CurrentParams.rainDir);
            emitter.SetGravity(DirectX::XMFLOAT3(0.0f, -9.8f, 0.0f));
        }

        // 砂嵐パーティクル
        if (m_SandComponent)
        {
            auto& emitter = m_SandComponent->GetEmitter();

            emitter.SetEmitRate(m_CurrentParams.sandEmitRate);
            emitter.SetLifeRange(m_CurrentParams.sandMinLife,
                m_CurrentParams.sandMaxLife);
            emitter.SetSpeedRange(m_CurrentParams.sandMinSpeed,
                m_CurrentParams.sandMaxSpeed);
            emitter.SetDirection(m_CurrentParams.sandDir);

            // 砂が少し上に舞うなら重力を弱めるなど
            emitter.SetGravity(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f));
        }
    }

private:
    WeatherType           m_CurrentWeather = WeatherType::Clear;

    WeatherParticleParams m_CurrentParams{};
    WeatherParticleParams m_SrcParams{};
    WeatherParticleParams m_DstParams{};

    float                 m_TransitionTime = 1.0f;
    float                 m_TransitionT = 1.0f;

    // 対応するコンポーネント
    std::array<ParticleComponent*, static_cast<size_t>(WeatherType::Weather_MAX)> m_Weeathers;
};

