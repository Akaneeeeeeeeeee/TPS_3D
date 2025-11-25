#pragma once
#include <cstddef>

// 前方宣言
struct aiAnimation;

/*
* @brief	アニメータークラス
* @detail	アニメーションの再生とブレンドを管理するクラス
* @remark	現在のアニメーションと次のアニメーションの保持と、ブレンド時間を管理だけを担当するクラス
* @auther	赤根　和樹
* @date		2025/11/23
*/
class Animator
{
public:
    // 初期状態
    Animator() = default;

    // 最初に再生するアニメをセット（Idle など）
    void SetInitialClip(aiAnimation* clip, float startTimeSec = 0.0f);

    // 「次にこのアニメへ遷移したい」ときに呼ぶ
    // nextClip が currentClip と同じなら、何もしない or 即切り替えにする
    void RequestTransition(aiAnimation* nextClip,
        float blendDurationSec);

    // 毎フレーム呼ぶ。dt は秒。
    void Update(const float dt);

    // ===== メッシュ側に渡すための情報取得 =====

    // 現在のアニメ（ブレンド中でも「from 側」）
    aiAnimation* GetCurrentClip() const { return m_CurrentClip; }

    // ブレンド先アニメ（ない場合は nullptr）
    aiAnimation* GetNextClip() const { return m_NextClip; }

	// 現在のフレーム位置（秒単位）
    float GetCurrentTimeSec() const { return m_CurrentTimeSec; }

	// 次のフレーム位置（秒単位）
    float GetNextTimeSec()    const { return m_NextTimeSec; }

    // ブレンド中かどうか
    bool IsBlending() const { return (m_NextClip != nullptr) && (m_BlendDuration > 0.0f); }

    // 0〜1 のブレンド係数（0: 完全に current, 1: 完全に next）
    float GetBlendAlpha() const;

private:
    aiAnimation* m_CurrentClip = nullptr;
    aiAnimation* m_NextClip = nullptr;

    // アニメ内の再生位置 [秒]
    float m_CurrentTimeSec = 0.0f;
    float m_NextTimeSec = 0.0f;

    // ブレンド時間 [秒]
    float m_BlendDuration = 0.0f;
    float m_BlendTimer = 0.0f;
};
