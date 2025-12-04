#pragma once
#include "system/commontypes.h"

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
    WeatherSystem();

    // ParticleComponent の登録 / 解除
    void Register(ParticleComponent* comp);
    void Unregister(ParticleComponent* comp);

    // 天候指定（t秒かけて遷移）
    void SetWeather(WeatherType type, float transitionSec);

    WeatherType GetWeather() const { return m_CurrentWeather; }

    // 毎フレーム呼ぶ
    void Update(float dt);

private:
    // パラメータ補間
    static WeatherParticleParams LerpParams(const WeatherParticleParams& a, const WeatherParticleParams& b, float t);

    // 現在の天候パラメータを、登録済みパーティクルへ反映
    void ApplyToParticles();

private:
    WeatherType           m_CurrentWeather = WeatherType::Clear;

    WeatherParticleParams m_CurrentParams{};
    WeatherParticleParams m_SrcParams{};
    WeatherParticleParams m_DstParams{};

    float                 m_TransitionTime = 1.0f;
    float                 m_TransitionT = 1.0f;

    // 登録されているパーティクルコンポーネント
    std::vector<ParticleComponent*> m_ParticleComponents;
};