#pragma once
#include "system/commontypes.h"
#include "system/RandomEngine.h"
#include <array>
#include <span>

// 天候タイプ
enum class WeatherType {
	Clear,          // 快晴
	LightRain,      // 小雨
	HeavyRain,      // 土砂降り
	Sandstorm,      // 砂嵐
	// Snow, Fog …を追加してもよい

	Weather_MAX
};

// 知覚影響パラメータ
struct PerceptionFactors
{
    float visibility = 1.0f;    // 視認性 (0～1)
    float hearing = 1.0f;       // 聴こえやすさ (0～1)
};

// ==============================
// 天候タイプ・天候パラメータ
// ==============================

struct WeatherParticleParams
{
    // --- 雨用 ---
	float             rainEmitRate = 0.0f;              // 1 秒あたりの放出数
	float             rainMinLife = 0.0f;               // 最小寿命
	float             rainMaxLife = 0.0f;               // 最大寿命
	float             rainMinSpeed = 0.0f;              // 最小速度
	float             rainMaxSpeed = 0.0f;              // 最大速度
	DirectX::XMFLOAT3 rainDir = { 0.0f, -1.0f, 0.0f };  // 進行方向（単位ベクトル）

    // --- 砂嵐用 ---
    float             sandEmitRate = 0.0f;
    float             sandMinLife = 0.0f;
    float             sandMaxLife = 0.0f;
    float             sandMinSpeed = 0.0f;
    float             sandMaxSpeed = 0.0f;
    DirectX::XMFLOAT3 sandDir = { 1.0f, 0.0f, 0.0f };
    // 砂嵐の発生高さレンジ（Emitter ローカル Y）
    float             sandMinHeight = 0.0f;
    float             sandMaxHeight = 0.0f;

    // --- 霧・空などに広げるためのパラメータ ---
    float             fogDensity = 0.0f;
    DirectX::XMFLOAT3 fogColor = { 1.0f, 1.0f, 1.0f };
};

// ==============================
// 太陽の状態
// ==============================
struct SunState
{
    // 0.0 ～ 24.0 のゲーム内時刻（時間）
    float   timeOfDayHours = 12.0f;

    // 1 日の長さ（現実何秒か）
    // 例: 600.0f → 現実 10 分でゲーム内 24 時間が 1 周
    float   dayLengthSec = 600.0f;

    // 「世界 → 太陽」方向ベクトル（空のどちら側に太陽があるか）
    Vector3 dirToSun = Vector3(0.0f, 1.0f, 0.0f);

    // 「太陽 → 世界」方向ベクトル（光が飛んでくる方向）
    // シェーダの平行光 Direction にそのまま渡す用
    Vector3 lightDir = Vector3(0.0f, -1.0f, 0.0f);

    // 太陽光の基準色（時間帯によって変化）
    Color   lightColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

    // 太陽光の強さ（0.0 ～ 1.0 程度）
    float   lightIntensity = 1.0f;
};

// ==============================
// 天候プリセット
// ==============================
inline WeatherParticleParams MakePreset(WeatherType type)
{
    WeatherParticleParams p{};
    using namespace DirectX;

    switch (type)
    {
    case WeatherType::Clear:
        p.rainEmitRate = 0.0f;
        p.sandEmitRate = 0.0f;
        p.fogDensity = 0.0f;
        break;

    case WeatherType::LightRain:
        p.rainEmitRate = 1000.0f;
        p.rainMinLife = 0.8f;
        p.rainMaxLife = 1.4f;
        p.rainMinSpeed = 80.0f;
        p.rainMaxSpeed = 120.0f;
        p.rainDir = XMFLOAT3(0.0f, -1.0f, 0.0f);

        p.sandEmitRate = 0.0f;
        p.fogDensity = 0.002f;
        p.fogColor = XMFLOAT3(0.7f, 0.7f, 0.8f);
        break;

    case WeatherType::HeavyRain:
        p.rainEmitRate = 8000.0f;
        p.rainMinLife = 1.0f;
        p.rainMaxLife = 1.8f;
        p.rainMinSpeed = 600.0f;
        p.rainMaxSpeed = 1000.0f;
        p.rainDir = XMFLOAT3(0.0f, -1.0f, 0.0f);

        p.sandEmitRate = 0.0f;
        p.fogDensity = 0.006f;
        p.fogColor = XMFLOAT3(0.6f, 0.6f, 0.7f);
        break;

    case WeatherType::Sandstorm:
        p.rainEmitRate = 0.0f;

        p.sandEmitRate = 5000.0f;
        p.sandMinLife = 5.0f;
        p.sandMaxLife = 8.0f;
        p.sandMinSpeed = 500.0f;
        p.sandMaxSpeed = 1000.0f;
        p.sandDir = XMFLOAT3(1.0f, 0.2f, 0.0f); // 斜め方向
        // Y = [0, 300] の範囲に出す
        p.sandMinHeight = 0.0f;
        p.sandMaxHeight = 300.0f;

        p.fogDensity = 0.01f;
        p.fogColor = XMFLOAT3(0.8f, 0.7f, 0.4f);
        break;
    }

    return p;
}

// ==============================
// 昼夜変化リスナーインターフェース
// ==============================
struct IDayNightListener
{
    virtual ~IDayNightListener() = default;
    virtual void OnDayNightChanged(bool isNight) = 0;
};

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

    // ---- パーティクル登録 ----
    void Register(ParticleComponent* comp);
    void Unregister(ParticleComponent* comp);

	// ---- 昼夜変化リスナー登録 ----
    void RegisterDayNightListener(IDayNightListener* l);
    void UnregisterDayNightListener(IDayNightListener* l);

    bool IsNight() const { return m_IsNight; }

    void SetNightHours(float onHour, float offHour)
    {
        m_NightOnHour = onHour;
        m_NightOffHour = offHour;
    }

    // ---- 天候指定（transitionSec 秒かけて補間） ----
    void SetWeather(WeatherType type, float transitionSec);
    WeatherType GetWeather() const { return m_CurrentWeather; }

    // ---- 太陽状態の取得・制御 ----
    const SunState& GetSun() const { return m_Sun; }

    // ゲーム内時刻を直接指定したいとき用（デバッグなど）
    void SetTimeOfDay(float hours) { m_Sun.timeOfDayHours = hours; }

    // 1 日の長さを変更（秒）
    void SetDayLength(float sec) { m_Sun.dayLengthSec = sec; }
	float GetDayLength() const { return m_Sun.dayLengthSec; }

    void Init(void);

    // ---- 毎フレーム更新 ----
    void Update(float dt);

    // ---- デバッグ描画 ----
    void DebugDrawParticles(void) const;
    void DebugDrawSun(void) const;
    void DebugImGui(void);    // ImGui用
    void DebugDrawRain(void) const;
    void DebugDrawSand(void) const;

    // DebugDrawParticles 用のカメラ行列
    void SetViewProjMatrices(Matrix4x4& viewMatrix, Matrix4x4& projMatrix)
    {
        view = viewMatrix;
        proj = projMatrix;
    }

    const PerceptionFactors& GetPerceptionFactors(void) const { return m_Perception; }
    float GetVisibilityFactor() const { return m_Perception.visibility; }
    float GetHearingFactor() const { return m_Perception.hearing; }

private:
    // ---- 天候補間（既存ロジック） ----
    static WeatherParticleParams LerpParams(
        const WeatherParticleParams& a,
        const WeatherParticleParams& b,
        float t
    );
    void ApplyToParticles();

	// ---- 知覚影響更新 ----
    void UpdatePerception(void);

    // ---- 太陽ロジック ----
    // 1) 時刻を進める
    void UpdateSunTime(float dt);
    // 2) 時刻から方向を求める
    void UpdateSunDirection();
    // 3) 時刻から色を求める
    Color ComputeSunBaseColor(float hours) const;
    void UpdateSunColorAndIntensity();
    // 4) SunState から LIGHT 構造体へ詰める
    void ApplyToLight();

    // 1日経過時の処理と次の天候決定
    void OnNewDay(void);                 // 1日経った瞬間に呼ぶ
    WeatherType ChooseNextWeather(void); // RandomEngine で次の天候を決める

    // ---- メンバ ----
    WeatherType           m_CurrentWeather = WeatherType::Clear;
    WeatherParticleParams m_CurrentParams{};
    WeatherParticleParams m_SrcParams{};
    WeatherParticleParams m_DstParams{};
    float                 m_TransitionTime = 1.0f;
    float                 m_TransitionT = 1.0f;

    std::vector<ParticleComponent*> m_ParticleComponents;

    SunState      m_Sun;
    PerceptionFactors m_Perception{};

    Matrix4x4 view = Matrix4x4::Identity;
    Matrix4x4 proj = Matrix4x4::Identity;

    // 天候専用の乱数
    RandomEngine m_Rng;

    bool  m_IsNight = false;
    float m_NightOnHour = 18.0f;
    float m_NightOffHour = 6.0f;

    std::vector<IDayNightListener*> m_DayNightListeners;

    bool ComputeIsNightByHour(float hours) const;
    void UpdateDayNightState(); // Update内で呼ぶ
};
