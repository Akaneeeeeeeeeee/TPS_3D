#pragma once
#include "system/Framework/SceneManager/SceneManager.h"
#include "SceneTransition.h"
#include "system/C3DShape.h"

/**
 * @brief フェード演出によるシーン遷移を行うクラス
 *
 * モードに応じて、フェードインのみ／フェードアウトのみ／その両方の演出が可能。
 * シーン切り替え時に黒い矩形を使って画面の明暗を調整する。
 */
class FadeTransition : public SceneTransition {
public:
    /**
    * @brief フェード演出のモード
     */
    enum class Mode {
        FadeInOnly,     // 明転
        FadeOutOnly,    // 暗転
		FadeInOut       // 暗転→明転
    };

    // duration_sec は 1.0f なら 1 秒など、SceneManager::Update の delta_time に合わせる
    explicit FadeTransition(float duration_sec, Mode mode = Mode::FadeInOut);

    /**
     * @brief フェード演出の開始処理
    *
    * @param nextSceneName 遷移先のシーン名
    */
    void Start(const std::string& nextSceneName) override;

    /**
     * @brief フェード演出の更新処理
    *
    * @param deltaTime 前フレームからの経過時間（マイクロ秒）
    */
    void Update(const float deltaTime) override;

    /**
    * @brief 黒フェード矩形の描画処理
    */
    void Draw(void) override;

    /**
     * @brief フェード演出の完了判定
    *
    * @return true フェード演出が終了している
    * @return false 演出中
    */
    bool IsFinished(void) const override;

    bool NeedsSceneChange(void) const override;
    void OnSceneChanged(void) override;

private:
    /**
    * @brief 現在のフェードフェーズ
    */
    enum class Phase {
        None,
        FadeOut,
        WaitSceneChange, // SceneManager に切り替えを要求して待っている状態
        FadeIn
    };

    Phase m_Phase = Phase::None;
    Mode m_Mode;

    Box m_Box;
	float m_Alpha = 0.0f;   // 0.0f ～ 1.0f
	float m_Duration;       // ミリ秒
	float m_Elapsed = 0;    // 経過時間（ミリ秒）
    std::string m_NextSceneName;
};
