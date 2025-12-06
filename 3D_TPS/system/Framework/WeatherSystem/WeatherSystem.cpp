#include "WeatherSystem.h"
#include "system/Framework/Component/Particle/ParticleComponent.h"
#include "SphereDrawer.h"
#include "renderer.h"
#include "DebugUI.h"

using DirectX::XMFLOAT3;

// ==============================
// コンストラクタ
// ==============================
WeatherSystem::WeatherSystem()
{
    // 初期状態は Clear
    m_CurrentParams = MakePreset(WeatherType::Clear);
    m_SrcParams = m_CurrentParams;
    m_DstParams = m_CurrentParams;
    m_TransitionTime = 1.0f;
    m_TransitionT = 1.0f;

    // 太陽の初期設定
    m_Sun.timeOfDayHours = 12.0f;   // 正午
    m_Sun.dayLengthSec = 60.0f;    // 現実 10 分で 1 日

    // 初期の方向（軽く南中しているイメージ）
    m_Sun.dirToSun = Vector3(0.0f, 1.0f, 0.0f);
    m_Sun.lightDir = -m_Sun.dirToSun;
    m_Sun.lightColor = Color(1, 1, 1, 1);
    m_Sun.lightIntensity = 1.0f;
}

// ==============================
// ParticleComponent 登録・解除
// ==============================
void WeatherSystem::Register(ParticleComponent* comp)
{
    if (!comp) return;

    auto it = std::find(m_ParticleComponents.begin(),
        m_ParticleComponents.end(),
        comp);
    if (it == m_ParticleComponents.end()) {
        m_ParticleComponents.push_back(comp);
    }
}

void WeatherSystem::Unregister(ParticleComponent* comp)
{
    if (!comp) return;

    auto it = std::remove(m_ParticleComponents.begin(),
        m_ParticleComponents.end(),
        comp);
    if (it != m_ParticleComponents.end()) {
        m_ParticleComponents.erase(it, m_ParticleComponents.end());
    }
}

// ==============================
// 天候変更（プリセット遷移開始）
// ==============================
void WeatherSystem::SetWeather(WeatherType type, float transitionSec)
{
    m_SrcParams = m_CurrentParams;
    m_DstParams = MakePreset(type);
    m_CurrentWeather = type;

    m_TransitionTime = std::max(transitionSec, 0.0001f);
    m_TransitionT = 0.0f;
}

// ==============================
// 天候パラメータ補間
// ==============================
WeatherParticleParams WeatherSystem::LerpParams(
    const WeatherParticleParams& a,
    const WeatherParticleParams& b,
    float t)
{
    auto lerp = [](float x, float y, float t)
        {
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

    // 向きは現状「天候ごとに決め打ち」でよいので a を採用
    r.rainDir = a.rainDir;
    r.sandDir = a.sandDir;

    r.fogDensity = lerp(a.fogDensity, b.fogDensity, t);
    r.fogColor.x = lerp(a.fogColor.x, b.fogColor.x, t);
    r.fogColor.y = lerp(a.fogColor.y, b.fogColor.y, t);
    r.fogColor.z = lerp(a.fogColor.z, b.fogColor.z, t);

    return r;
}

// ==============================
// パーティクルへの反映
// ==============================
void WeatherSystem::ApplyToParticles()
{
    float   emitRate = 0.0f;
    float   minLife = 0.0f;
    float   maxLife = 0.0f;
    float   minSpeed = 0.0f;
    float   maxSpeed = 0.0f;
    XMFLOAT3 dir = { 0.0f, -1.0f, 0.0f };
    XMFLOAT3 gravity = { 0.0f, -9.8f, 0.0f };

    float   spawnHalfWidth = 0.0f;
    float   spawnHalfDepth = 0.0f;
    float   spawnHeight = 0.0f;
    size_t  maxParticles = 2000;

    // 雨が有効
    if (m_CurrentParams.rainEmitRate > 0.0f)
    {
        emitRate = m_CurrentParams.rainEmitRate;
        minLife = m_CurrentParams.rainMinLife;
        maxLife = m_CurrentParams.rainMaxLife;
        minSpeed = m_CurrentParams.rainMinSpeed;
        maxSpeed = m_CurrentParams.rainMaxSpeed;
        dir = m_CurrentParams.rainDir;
        gravity = XMFLOAT3(0.0f, -9.8f, 0.0f);

        spawnHalfWidth = 800.0f;
        spawnHalfDepth = 800.0f;
        spawnHeight = 0.0f;
        maxParticles = 15000;
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
        gravity = XMFLOAT3(0.0f, -1.0f, 0.0f);

        spawnHalfWidth = 1000.0f;
        spawnHalfDepth = 1000.0f;
        spawnHeight = 0.0f;
        maxParticles = 12000;
    }

    for (auto* comp : m_ParticleComponents)
    {
        if (!comp) continue;

        auto& emitter = comp->GetEmitter();

        emitter.SetEmitRate(emitRate);
        emitter.SetLifeRange(minLife, maxLife);
        emitter.SetSpeedRange(minSpeed, maxSpeed);
        emitter.SetDirection(dir);
        emitter.SetGravity(gravity);

        emitter.SetSpawnAreaXZ(spawnHalfWidth, spawnHalfDepth);
        emitter.SetSpawnHeight(spawnHeight);
        emitter.SetMaxParticles(maxParticles);
    }
}

// ==============================
// 太陽：1) 時刻を進める
// ==============================
void WeatherSystem::UpdateSunTime(float dt)
{
    if (m_Sun.dayLengthSec <= 0.0f)
        m_Sun.dayLengthSec = 1.0f;

    // dayLengthSec 秒で 24 時間進むように加算
    m_Sun.timeOfDayHours += (dt / m_Sun.dayLengthSec) * 24.0f;

    // 0～24 に正規化
    if (m_Sun.timeOfDayHours >= 24.0f)
        m_Sun.timeOfDayHours = fmodf(m_Sun.timeOfDayHours, 24.0f);
    if (m_Sun.timeOfDayHours < 0.0f)
        m_Sun.timeOfDayHours += 24.0f;
}

// ==============================
// 太陽：2) 時刻から方向を求める
// ==============================
void WeatherSystem::UpdateSunDirection()
{
    // 0～24 → 0～2π に変換し、π/2 ずらして「6時が地平線」になるよう調整
    float angle = (m_Sun.timeOfDayHours / 24.0f) * DirectX::XM_2PI
        - DirectX::XM_PIDIV2;

    Vector3 dirToSun;
    dirToSun.x = 0.0f;
    dirToSun.y = std::sinf(angle); // 高さ
    dirToSun.z = std::cosf(angle); // 前後方向
    dirToSun.Normalize();

    // 空に向かう向き
    m_Sun.dirToSun = dirToSun;
    // 光が飛んでくる向き（シェーダ用）
    m_Sun.lightDir = -dirToSun;
}

// ==============================
// 太陽：3) 時刻から基準色を求める
// ==============================
Color WeatherSystem::ComputeSunBaseColor(float hours) const
{
    const Color nightColor = Color(0.05f, 0.07f, 0.2f);
    const Color morningColor = Color(1.0f, 0.4f, 0.2f);
    const Color noonColor = Color(1.0f, 1.0f, 1.0f);
    const Color sunsetColor = Color(1.0f, 0.5f, 0.2f);

    if (hours < 4.0f) {
        // 真夜中
        return nightColor;
    }
    else if (hours < 7.0f) {
        // 夜 → 朝焼け
        float k = (hours - 4.0f) / 3.0f;
        return nightColor + (morningColor - nightColor) * k;
    }
    else if (hours < 16.0f) {
        // 朝焼け → 昼
        float k = (hours - 7.0f) / 9.0f;
        return morningColor + (noonColor - morningColor) * k;
    }
    else if (hours < 19.0f) {
        // 昼 → 夕焼け
        float k = (hours - 16.0f) / 3.0f;
        return noonColor + (sunsetColor - noonColor) * k;
    }
    else {
        // 夕焼け → 夜
        float k = (hours - 19.0f) / 5.0f;
        return sunsetColor + (nightColor - sunsetColor) * k;
    }
}

void WeatherSystem::UpdateSunColorAndIntensity()
{
    // 基準色（時間帯に応じた色）
    Color base = ComputeSunBaseColor(m_Sun.timeOfDayHours);

    // 高さに応じて強度を決める
    // dirToSun.y が高いほど明るくする。0 以下は夜として 0。
    float heightFactor = std::clamp(m_Sun.dirToSun.y, 0.0f, 1.0f);
    float intensity = std::pow(heightFactor, 0.5f);

    m_Sun.lightColor = base;
    m_Sun.lightIntensity = intensity;
}

// ==============================
// 太陽：4) SunState → LIGHT へ詰める
// ==============================
void WeatherSystem::ApplyToLight()
{
    // 今のライトをベースに上書き
    LIGHT light = Renderer::GetLight();

    light.Enable = true;
    light.Direction = Vector4(
        m_Sun.lightDir.x,
        m_Sun.lightDir.y,
        m_Sun.lightDir.z,
        0.0f);

    // 太陽そのものの Diffuse 成分
    Color sun = m_Sun.lightColor * m_Sun.lightIntensity;
    float I = m_Sun.lightIntensity;

    // 簡易 GI：空・地面からの間接光を足す
    Color skyColor = Color(0.3f, 0.4f, 0.5f) * (0.5f + 0.5f * I);
    Color groundColor = Color(0.1f, 0.1f, 0.05f) * (1.0f - I);

    Color totalDiffuse = sun + skyColor + groundColor;

    light.Diffuse = totalDiffuse;
    light.Ambient = Color(0.2f, 0.2f, 0.2f) * (0.5f + 0.5f * I)
        + groundColor * 0.2f;

    Renderer::SetLight(light);
}

void WeatherSystem::Init(void)
{
	DebugUI::RedistDebugFunction([this]() { this->DebugImGui(); });
}

// ==============================
// 毎フレーム Update
// ==============================
void WeatherSystem::Update(float dt)
{
    // ---- 1) 天候パラメータの遷移 ----
    if (m_TransitionT < 1.0f)
    {
        m_TransitionT += dt / m_TransitionTime;
        if (m_TransitionT > 1.0f) m_TransitionT = 1.0f;

        m_CurrentParams = LerpParams(m_SrcParams, m_DstParams, m_TransitionT);
    }

    // パーティクルへ反映
    ApplyToParticles();

    // ---- 2) 太陽・ライト更新 ----
    UpdateSunTime(dt);           // 時刻を進める
    UpdateSunDirection();        // 時刻 → 方向
    UpdateSunColorAndIntensity();// 方向＋時間 → 色・強さ
    ApplyToLight();              // SunState → Renderer::SetLight

    // 将来的に Fog / Sky / PBR 用定数バッファもここで更新
}

// ==============================
// デバッグ：パーティクル描画
// ==============================
void WeatherSystem::DebugDrawParticles() const
{
    Color rainColor(0.4f, 0.4f, 1.0f, 1.0f);
    Color sandColor(0.9f, 0.8f, 0.5f, 1.0f);
    Color col(1.0f, 1.0f, 1.0f, 1.0f);

    if (m_CurrentParams.rainEmitRate > 0.0f) {
        col = rainColor;
    }
    else if (m_CurrentParams.sandEmitRate > 0.0f) {
        col = sandColor;
    }

    constexpr float PARTICLE_RADIUS = 0.5f;

    size_t totalCount = 0;
    for (auto* comp : m_ParticleComponents)
    {
        if (!comp || !comp->GetIsValid()) continue;
        totalCount += comp->GetEmitter().GetParticles().size();
    }
    if (totalCount == 0) return;

    std::vector<Vector3> centers;
    centers.reserve(totalCount);

    for (auto* comp : m_ParticleComponents)
    {
        if (!comp || !comp->GetIsValid()) continue;

        const auto& particles = comp->GetEmitter().GetParticles();
        for (const auto& p : particles)
        {
            centers.emplace_back(p.pos.x, p.pos.y, p.pos.z);
        }
    }

    SphereInstancedDrawerDraw(
        view,
        proj,
        centers,
        PARTICLE_RADIUS,
        col);
}

// ==============================
// デバッグ：太陽の可視化
// ==============================
void WeatherSystem::DebugDrawSun() const
{
    // カメラ原点から見て「空の一点」に見えるよう、大きさと距離を分けて設定
    constexpr float SUN_DISTANCE = 5000.0f; // 太陽までの距離
    constexpr float SUN_RADIUS = 300.0f;  // 見た目の半径

    Vector3 dir = m_Sun.dirToSun;
    if (dir.LengthSquared() < 1e-6f) return;
    dir.Normalize();

    // 原点から dir 方向に SUN_DISTANCE 離れた場所に太陽を置く
    Vector3 sunPos = dir * SUN_DISTANCE;

    Matrix4x4 scale = Matrix4x4::CreateScale(SUN_RADIUS, SUN_RADIUS, SUN_RADIUS);
    Matrix4x4 trans = Matrix4x4::CreateTranslation(sunPos.x, sunPos.y, sunPos.z);
    Matrix4x4 world = scale * trans;

    // 太陽の色は SunState の色・強さを使用
    Color sunColor = m_Sun.lightColor * m_Sun.lightIntensity;

    Renderer::SetDepthEnable(false);
    SphereDrawerDraw(world, sunColor);
    Renderer::SetDepthEnable(true);
}

// WeatherSystem.cpp

void WeatherSystem::DebugImGui()
{
#ifdef _DEBUG
    // ウィンドウ開始
    if (!ImGui::Begin("Weather / Sun System"))
    {
        ImGui::End();
        return;
    }

    // -----------------------------
    // 1. 時間・太陽まわり
    // -----------------------------
    ImGui::Text("Time / Sun");
    ImGui::Separator();

    // ゲーム内時間（0～24 時間）
    float hours = m_Sun.timeOfDayHours;
    if (ImGui::SliderFloat("TimeOfDay [hours]", &hours, 0.0f, 24.0f))
    {
        // スライダーから直接変更したい場合
        m_Sun.timeOfDayHours = hours;
    }

    // 1日の長さ（秒）
    ImGui::SliderFloat("DayLength [sec]", &m_Sun.dayLengthSec, 10.0f, 3600.0f);

    // 現在の SunState から情報を表示
    ImGui::Separator();
    ImGui::Text("SunState");

    ImGui::Text("dirToSun  = (%.3f, %.3f, %.3f)",
        m_Sun.dirToSun.x, m_Sun.dirToSun.y, m_Sun.dirToSun.z);
    ImGui::Text("lightDir  = (%.3f, %.3f, %.3f)",
        m_Sun.lightDir.x, m_Sun.lightDir.y, m_Sun.lightDir.z);

    ImGui::Text("lightColor= (%.3f, %.3f, %.3f)",
        m_Sun.lightColor.R(), m_Sun.lightColor.G(), m_Sun.lightColor.B());
    ImGui::Text("intensity = %.3f", m_Sun.lightIntensity);

    // -----------------------------
    // 2. 天候・フォグ
    // -----------------------------
    ImGui::Separator();
    ImGui::Text("Weather");
    ImGui::Separator();

    // 天候タイプをコンボボックスで選択
    static const char* s_WeatherNames[] = {
        "Clear",
        "LightRain",
        "HeavyRain",
        "Sandstorm",
    };

    int weatherIndex = static_cast<int>(m_CurrentWeather);
    if (ImGui::Combo("WeatherType", &weatherIndex,
        s_WeatherNames,
        IM_ARRAYSIZE(s_WeatherNames)))
    {
        // 天候変更。遷移時間は仮で 3 秒
        WeatherType newType = static_cast<WeatherType>(weatherIndex);
        SetWeather(newType, 3.0f);
    }

    // 現在パラメータの fog を確認・調整
    ImGui::Separator();
    ImGui::Text("Fog (CurrentParams)");

    ImGui::SliderFloat("FogDensity", &m_CurrentParams.fogDensity, 0.0f, 0.05f);

    float fogCol[3] = {
        m_CurrentParams.fogColor.x,
        m_CurrentParams.fogColor.y,
        m_CurrentParams.fogColor.z
    };
    if (ImGui::ColorEdit3("FogColor", fogCol))
    {
        m_CurrentParams.fogColor.x = fogCol[0];
        m_CurrentParams.fogColor.y = fogCol[1];
        m_CurrentParams.fogColor.z = fogCol[2];
    }

    // 現在の雨・砂嵐パラメータもざっくり見たい場合
    ImGui::Separator();
    ImGui::Text("Particles (CurrentParams)");
    ImGui::Text("Rain EmitRate = %.1f", m_CurrentParams.rainEmitRate);
    ImGui::Text("Sand EmitRate = %.1f", m_CurrentParams.sandEmitRate);

    ImGui::End();
#endif
}
