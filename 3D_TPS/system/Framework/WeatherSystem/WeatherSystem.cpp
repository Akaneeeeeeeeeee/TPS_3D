#include "WeatherSystem.h"
#include "system/Framework/Component/Particle/ParticleComponent.h"
#include "SphereDrawer.h"
#include "renderer.h"
#include "DebugUI.h"
#include "CylinderDrawer.h"

using DirectX::XMFLOAT3;

// ==============================
// コンストラクタ
// ==============================
WeatherSystem::WeatherSystem()
    : m_Rng(RandomEngine::tls().stream("WeatherSystem")) // ★ weather 用サブストリーム
{
	// 初期状態は晴れ
    m_CurrentParams = MakePreset(WeatherType::Clear);
    m_SrcParams = m_CurrentParams;
    m_DstParams = m_CurrentParams;
    m_TransitionTime = 1.0f;
    m_TransitionT = 1.0f;

    // 太陽の初期設定
    m_Sun.timeOfDayHours = 12.0f;    // 正午スタート

    // 1日の長さを「5分 = 300秒」にしておく
    // SetDayLength() や ImGui のスライダーから変更可能
    m_Sun.dayLengthSec = 300.0f;

    // 初期の方向
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
// 1日経過時の処理
// ==============================
void WeatherSystem::OnNewDay()
{
    // その日の天候を一度だけ決める
    WeatherType next = ChooseNextWeather();

    // 同じ天候を引くことも自然なので許容
    // 必ず変えたいなら if (next == m_CurrentWeather) ... で再抽選してもよい
    if (next != m_CurrentWeather)
    {
        // 日付が変わったタイミングで数秒かけて切り替える
        constexpr float TRANSITION_SEC = 3.0f;
        SetWeather(next, TRANSITION_SEC);
    }
}

// ==============================
// 次の日の天候を乱数で決める
// ==============================
WeatherType WeatherSystem::ChooseNextWeather()
{
    using WT = WeatherType;

    // WeatherType::Weather_MAX 個ぶんの重み
    std::array<double, static_cast<size_t>(WT::Weather_MAX)> w{};

	// 現在の天候に応じて、次の天候の重みを設定
    switch (m_CurrentWeather)
    {
    case WT::Clear:
        // 晴れが多め、たまに小雨、まれに土砂降りや砂嵐
        w[static_cast<size_t>(WT::Clear)] = 5.0;
        w[static_cast<size_t>(WT::LightRain)] = 3.0;
        w[static_cast<size_t>(WT::HeavyRain)] = 1.0;
        w[static_cast<size_t>(WT::Sandstorm)] = 1.0;
        break;

    case WT::LightRain:
        // 小雨が続く or 晴れる or 強い雨
        w[static_cast<size_t>(WT::Clear)] = 3.0;
        w[static_cast<size_t>(WT::LightRain)] = 4.0;
        w[static_cast<size_t>(WT::HeavyRain)] = 2.0;
        w[static_cast<size_t>(WT::Sandstorm)] = 1.0;
        break;

    case WT::HeavyRain:
        // 土砂降りが連続すると重いので、軽い雨か晴れに寄せる
        w[static_cast<size_t>(WT::Clear)] = 4.0;
        w[static_cast<size_t>(WT::LightRain)] = 4.0;
        w[static_cast<size_t>(WT::HeavyRain)] = 1.0;
        w[static_cast<size_t>(WT::Sandstorm)] = 1.0;
        break;

    case WT::Sandstorm:
        // 砂嵐はレアにして、次の日は晴れか小雨に戻りやすく
        w[static_cast<size_t>(WT::Clear)] = 4.0;
        w[static_cast<size_t>(WT::LightRain)] = 3.0;
        w[static_cast<size_t>(WT::HeavyRain)] = 1.0;
        w[static_cast<size_t>(WT::Sandstorm)] = 1.0;
        break;

    default:
        // 万一未設定なら全部「晴れ」
        w[static_cast<size_t>(WT::Clear)] = 1.0;
        break;
    }

    // RandomEngine の重み付きインデックスで 0..Weather_MAX-1 を選ぶ
    std::span<const double> span(w.data(), w.size());
    size_t idx = m_Rng.weightedIndex(span);

    return static_cast<WeatherType>(idx);
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

    // 高さレンジ（Emitter ローカル Y）
    float   spawnMinY = 0.0f;
    float   spawnMaxY = 0.0f;

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

        // 雨はエミッタの真上あたりにまとめて出すイメージなら 0〜0 でもよい
        spawnMinY = 0.0f;
        spawnMaxY = 0.0f;
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
        // プリセットで決めた高さレンジをそのまま使う
        spawnMinY = m_CurrentParams.sandMinHeight;
        spawnMaxY = m_CurrentParams.sandMaxHeight;

        // XZ方向を広げる
        spawnHalfWidth = 2000.0f;   // 1000 → 2000
        spawnHalfDepth = 2000.0f;

        // 高さ方向の厚みを持たせたいなら、spawnHeight も上げる
        //  SetSpawnHeight の解釈は実装次第なので（推測です）、
        //   「高さレンジ」として使っているなら 200〜500 ぐらいを試す。
        spawnHeight = 300.0f;    // 0 → 300（推測です）

        // 最大粒子数も増やして間引かれないように
        maxParticles = 300000;     // 12000 → 30000
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
        // 「高さレンジ」を渡す新しい API を呼ぶ
        emitter.SetSpawnHeightRange(spawnMinY, spawnMaxY);
        emitter.SetMaxParticles(maxParticles);
    }
}

// ==============================
// 太陽：1) 太陽時刻更新
// ==============================
void WeatherSystem::UpdateSunTime(float dt)
{
    if (m_Sun.dayLengthSec <= 0.0f)
        m_Sun.dayLengthSec = 1.0f;

    // 更新前のゲーム内時刻を保存
    float prevHours = m_Sun.timeOfDayHours;

    // dayLengthSec 秒で 24 時間進む
    m_Sun.timeOfDayHours += (dt / m_Sun.dayLengthSec) * 24.0f;

    // 0～24 に正規化
    if (m_Sun.timeOfDayHours >= 24.0f)
        m_Sun.timeOfDayHours = std::fmod(m_Sun.timeOfDayHours, 24.0f);
    if (m_Sun.timeOfDayHours < 0.0f)
        m_Sun.timeOfDayHours += 24.0f;

    // 24 → 0 に回り込んだら「1日経過」
    // （prev=23.9 → now=0.1 のようなケース）
    if (m_Sun.timeOfDayHours < prevHours)
    {
        OnNewDay();
    }
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

	// ---- 3) 知覚影響更新 ----
	UpdatePerception();

    // 将来的に Fog / Sky / PBR 用定数バッファもここで更新
}

void WeatherSystem::UpdatePerception(void)
{
    // 1) 時刻による明るさ（0～1）
    float t = (m_Sun.timeOfDayHours - 6.0f) / 12.0f; // 6～18時を 0～1
    t = std::clamp(t, 0.0f, 1.0f);

    float angle = t * PI;
    float noonFactor = std::sin(angle);   // 昼 1.0, 夜 0.0
    noonFactor = std::max(0.0f, noonFactor);

    float timeVisibility = 0.2f + 0.8f * noonFactor;
    // → 真夜中 0.2, 正午 1.0 くらいのイメージ

    // 2) 天候によるペナルティ
    float weatherVisibility = 1.0f;
    float weatherHearing = 1.0f;

    switch (m_CurrentWeather)
    {
    case WeatherType::Clear:
        weatherVisibility = 1.0f;
        weatherHearing = 1.0f;
        break;
    case WeatherType::LightRain:
        weatherVisibility = 0.8f;
        weatherHearing = 0.9f;
        break;
    case WeatherType::HeavyRain:
        weatherVisibility = 0.5f;
        weatherHearing = 0.6f; // 雨音で聞こえづらい
        break;
    case WeatherType::Sandstorm:
        weatherVisibility = 0.3f;
        weatherHearing = 0.8f;
        break;
    default:
        break;
    }

    m_Perception.visibility = std::clamp(timeVisibility * weatherVisibility, 0.1f, 1.0f);
    m_Perception.hearing = std::clamp(weatherHearing, 0.1f, 1.0f);
}

// ==============================
// デバッグ：パーティクル描画
// ==============================
void WeatherSystem::DebugDrawParticles(void) const
{
    if (m_CurrentParams.rainEmitRate > 0.0f)
    {
        DebugDrawRain();
    }
    else if (m_CurrentParams.sandEmitRate > 0.0f)
    {
        DebugDrawSand();
    }
}


void WeatherSystem::DebugDrawRain() const
{
    // 半径（太さ）と色は調整用
    constexpr float RAIN_RADIUS = 0.5f;
    Color rainColor(0.6f, 0.6f, 1.0f, 1.0f);

    std::vector<RainInstance> instances;

    for (auto* comp : m_ParticleComponents)
    {
        if (!comp || !comp->GetIsValid()) continue;

        const auto& particles = comp->GetEmitter().GetParticles();
        instances.reserve(instances.size() + particles.size());

        for (const auto& p : particles)
        {
            // 速度ベクトルから長さを決める（簡易）
            Vector3 vel(p.vel.x, p.vel.y, p.vel.z);
            float speed = vel.Length();

            RainInstance ri;
            ri.pos = Vector3(p.pos.x, p.pos.y, p.pos.z);
            ri.length = std::clamp(speed * 0.02f, 10.0f, 200.0f); // 最小/最大を制限

            instances.push_back(ri);
        }
    }

    if (instances.empty()) return;

    RainInstancedDrawerDraw(
        view,
        proj,
        instances,
        RAIN_RADIUS,
        rainColor);
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

void WeatherSystem::DebugDrawSand() const
{
    Color sandColor(0.9f, 0.8f, 0.5f, 1.0f);
    constexpr float SAND_RADIUS = 0.5f;   // かなり小さめに

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
        SAND_RADIUS,
        sandColor);
}


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
