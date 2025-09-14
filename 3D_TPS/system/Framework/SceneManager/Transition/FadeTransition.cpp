#include    "system/Framework/Application/Entry/main.h"
#include    "system/Framework/SceneManager/SceneManager.h"
#include    "FadeTransition.h"
#include    "system/BoxDrawer.h"

/**
 * @brief フェード遷移の開始処理
 *
 * モードに応じてフェーズを設定し、必要に応じて先にシーンを切り替える。
 *
 * @param nextSceneName 遷移先のシーン名
 */
void FadeTransition::start(const std::string& nextSceneName) {
    m_nextScene = nextSceneName;
    m_elapsed = 0;

    switch (m_mode) {
    case Mode::FadeOutOnly:
        m_phase = Phase::FadeOut;
        m_alpha = 0.0f;
        break;
    case Mode::FadeInOnly:
		// ここで切り替えると遷移が二回行われるので、updateで切り替える
        //m_pSceneManager->ChangeScene(m_nextScene); // 先にシーン変更
        m_phase = Phase::FadeIn;
        m_alpha = 1.0f;
        break;
    case Mode::FadeInOut:
        m_phase = Phase::FadeOut;
        m_alpha = 0.0f;
        break;
    }
}

/**
 * @brief フェードの進行を更新する
 *
 * 経過時間に基づいて透明度（m_alpha）を変化させる。
 * フェードアウト終了後はシーンを切り替え、フェードインに移行する。
 *
 * @param deltaTime 前フレームからの経過時間（マイクロ秒）
 */
void FadeTransition::update(uint64_t deltaTime) {
	// 累積経過時間を更新
    m_elapsed += deltaTime;

    switch (m_phase) {
    case Phase::FadeOut:
		// フェードアウト完了後にシーン切り替え
        m_alpha = static_cast<float>(m_elapsed) / m_duration;
        if (m_elapsed >= m_duration) {
            m_alpha = 1.0f;
            m_elapsed = 0;

            if (m_mode == Mode::FadeOutOnly) {
                m_phase = Phase::None;
            }
            else {
				// ここが重すぎるので非同期処理が必要
                m_pSceneManager->ChangeScene(m_nextScene);
                m_phase = Phase::FadeIn;
            }
        }
        break;

    case Phase::FadeIn:
        // FadeInOnlyの場合はここでシーン切り替えを行う
        if (m_mode == Mode::FadeInOnly && m_elapsed == deltaTime) {
            // 最初のフレームだけ切り替え
            m_pSceneManager->ChangeScene(m_nextScene);
        }

        m_alpha = 1.0f - static_cast<float>(m_elapsed) / m_duration;
        if (m_elapsed >= m_duration) {
            m_alpha = 0.0f;
            m_phase = Phase::None;
        }
        break;

    default:
        break;
    }
}

/**
 * @brief フェード用の黒い矩形を画面に描画する
 *
 * 現在の透明度に応じた黒い全画面オーバーレイを表示し、自然なフェード効果を演出する。
 */
void FadeTransition::draw() {
    if (m_phase != Phase::None) {
        /*BoxDrawerDraw(
            SCREEN_WIDTH, SCREEN_HEIGHT, 0,
            Color(0, 0, 0, m_alpha),
            0.0f, 0.0f, 0
        );*/
        BoxDrawerDraw(
            SCREEN_WIDTH, SCREEN_HEIGHT, 0,
            Color(0, 0, 0, m_alpha),
            SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0
        );
    }
}

/**
 * @brief フェード演出が完了したかを判定する
 *
 * 現在のフェーズが None になっていれば演出完了。
 *
 * @return true フェードが完了している
 * @return false まだ演出中である
 */
bool FadeTransition::isFinished() const {
    return m_phase == Phase::None;
}
