#include "SoundWaveVisualizer.h"
#include "system/LineDrawer.h"

void SoundWaveVisualizer::OnEmit(const WorldSoundEvent& ev)
{
    // 足音だけを対象にしたい場合
    if (ev.Type != SoundType::Footstep)
        return;

    Wave w{};
    w.center = ev.Position;
    w.maxRadius = ev.Radius;     // 敵が聞こえる範囲と同じにしておくと分かりやすい
    w.currentRadius = 0.0f;
    w.lifeTime = 0.5f;         // 0.5秒で消える波（好みで調整）
    w.elapsed = 0.0f;
    w.loudness = ev.Loudness;

    // loudness に応じて色を変えたいならここで決める
    // 例：常に白
    w.baseColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

    m_Waves.push_back(w);
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
        const float y = w.center.y + 5.0f; // 地面から少し浮かせる

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

            Vector3 dir = p1 - p0;
            float   len = dir.Length();
            if (len <= 0.0f) continue;
            dir /= len; // 正規化

            // 1本の線を描画
            LineDrawerDraw(len, p0, dir, col);
        }
    }
}
