#include "SoundWaveVisualizer.h"
#include "system/LineDrawer.h"
#include "system/Framework/WeatherSystem/WeatherSystem.h"

void SoundWaveVisualizer::OnEmit(const WorldSoundEvent& ev)
{
    // 足音だけを対象にしたい
    if (ev.Type != SoundType::Footstep && ev.Type != SoundType::StoneImpact) { return; }
    
    // 敵の足音は可視化しない
    if (ev.Type == SoundType::Footstep && ev.Emitter == SoundEmitterKind::Enemy) { return; }

    // ---- 天候・時間による聴覚係数 ----
    float hearingFactor = 1.0f;
    if (m_pWeather)
    {
        hearingFactor = m_pWeather->GetHearingFactor();
    }

    Wave w{};
    w.center = ev.Position;

    // 敵が「実際に聞こえる範囲」と同じにしたければ EnemyHearing と整合させる
    // → ev.Radius * hearingFactor にしておく
    w.maxRadius = ev.Radius * hearingFactor;

    w.currentRadius = 0.0f;
    w.lifeTime = 0.5f;
    w.elapsed = 0.0f;

    // 音量も天候で減衰させた値を可視化に使う
    w.loudness = ev.Loudness * hearingFactor;

    // 色は天候で少し変えてもよい（例として HeavyRain で少し青くする）
    Color base = Color(1.0f, 1.0f, 1.0f, 1.0f);
    if (m_pWeather)
    {
        switch (m_pWeather->GetWeather())
        {
        case WeatherType::HeavyRain:
            base = Color(0.7f, 0.7f, 1.0f, 1.0f);
            break;
        case WeatherType::Sandstorm:
            base = Color(1.0f, 0.9f, 0.6f, 1.0f);
            break;
        default:
            break;
        }
    }
    w.baseColor = base;

    m_Waves.push_back(w);
}

void SoundWaveVisualizer::OnWorldSound(const WorldSoundEvent& ev)
{
    OnEmit(ev);
}

void SoundWaveVisualizer::Update(float dt)
{
    for (auto& w : m_Waves)
    {
        w.elapsed += dt;
        float t = w.elapsed / w.lifeTime;
        if (t > 1.0f) t = 1.0f;

        // 0 → maxRadius まで広げる
        w.currentRadius = w.maxRadius * t;
    }

    // 寿命切れを削除
    m_Waves.erase(
        std::remove_if(
            m_Waves.begin(), m_Waves.end(),
            [](const Wave& w) { return w.elapsed >= w.lifeTime; }
        ),
        m_Waves.end()
    );
}

void SoundWaveVisualizer::DrawWorld(void)
{
    if (m_Waves.empty()) return;

    constexpr int SEGMENT = 64; // 円の分割数（多すぎるとコスト増）

    for (const auto& w : m_Waves)
    {
        if (w.currentRadius <= 0.0f)
            continue;

        // 時間経過で透明にする
        float t = w.elapsed / w.lifeTime;
        if (t > 1.0f) t = 1.0f;

        float alpha = 1.0f - t;

        // loudness を 0〜1 に正規化して強さに使う
        float normL = (m_MaxLoudness > 0.0f) ? (w.loudness / m_MaxLoudness) : 1.0f;
        if (normL > 1.0f) normL = 1.0f;

        Color col = w.baseColor;
        col.w *= alpha * normL; // 時間・音量で透明度を変える

        // 線の太さも loudness 依存にする例
        float baseWidth = 3.0f;
        float width = baseWidth * (0.5f + 0.5f * normL);
        SetLineWidth(width);   // ここで太さをセット

        // 円を SEGMENT 個の線分に分けて描画
        const float r = w.currentRadius;
        const float y = w.center.y + 50.0f; // 地面から少し浮かせる

        std::vector<LineInstanceParam> lines;
        lines.reserve(SEGMENT);

        for (int i = 0; i < SEGMENT; ++i)
        {
            float a0 = (2.0f * PI) * (i / static_cast<float>(SEGMENT));
            float a1 = (2.0f * PI) * ((i + 1) / static_cast<float>(SEGMENT));

            Vector3 p0(
                w.center.x + std::cos(a0) * r,
                y,
                w.center.z + std::sin(a0) * r
            );
            Vector3 p1(
                w.center.x + std::cos(a1) * r,
                y,
                w.center.z + std::sin(a1) * r
            );

            // 長さ0を避ける
            Vector3 d = p1 - p0;
            if (d.LengthSquared() <= 1e-6f) continue;

            LineInstanceParam inst{};
            inst.start = p0;
            inst.end = p1;
            inst.color = col;

            lines.push_back(inst);
        }
        // まとめて描画
        LineInstancedDrawerDraw(lines);
    }
}
