#pragma once
#include <vector>
#include "commontypes.h"
#include "system/Framework/NonCopyable/Singleton_Template.h"
#include "system/Sound/WorldSoundEvent.h"

class SoundWaveVisualizer : public Singleton<SoundWaveVisualizer>
{
public:
    friend class Singleton<SoundWaveVisualizer>;

    // 毎フレーム冒頭で呼ぶ（消滅判定など）
    void Update(float dt);

    // 世界座標での描画（プレイヤーの周りに円）
    void DrawWorld(void);

    // 画面 UI として波形を出したい場合（任意）
    void DrawUI(void);

    // SoundManager から呼ぶ
    void OnEmit(const WorldSoundEvent& ev);

    // loudness の最大値（正規化用）を設定
    void SetMaxLoudness(float v) { m_MaxLoudness = v; }

private:
    SoundWaveVisualizer() = default;

    struct Wave
    {
        Vector3 center;       // 波の中心（音が出た位置）
        float   maxRadius;    // この半径まで広がったら消える（ev.Radius）
        float   currentRadius;
        float   lifeTime;     // 生存時間 [秒]
        float   elapsed;      // 経過時間 [秒]
        float   loudness;     // ev.Loudness
        Color   baseColor;    // 元の色
    };

    std::vector<Wave> m_Waves;

    // 画面 UI 用（必要なら）
    std::vector<float> m_Samples;
    int   m_WriteIndex = 0;
    float m_MaxLoudness = 1.0f;
    float m_DecayPerSec = 1.5f;

    //void PushSample(float loudness);
};
