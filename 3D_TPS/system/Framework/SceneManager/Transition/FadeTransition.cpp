#include    "system/Framework/Application/Entry/main.h"
#include    "system/Framework/SceneManager/SceneManager.h"
#include    "FadeTransition.h"
#include    "system/BoxDrawer.h"

/**
* @brief コンストラクタ
* @param durationMs フェード時間（ミリ秒）
* @param mode フェードモード（デフォルトは FadeInOut）
*/
FadeTransition::FadeTransition(float duration_sec, Mode mode)
	: m_Mode(mode)
	, m_Duration(duration_sec)
	, m_Box(SCREEN_WIDTH, SCREEN_HEIGHT, 0)
{
	//BoxDrawerInit();
}

/**
 * @brief フェード遷移の開始処理
 *
 * モードに応じてフェーズを設定し、必要に応じて先にシーンを切り替える。
 *
 * @param nextSceneName 遷移先のシーン名
 */
void FadeTransition::Start(const std::string& nextSceneName) {
	m_NextSceneName = nextSceneName;
	m_Elapsed = 0;

	switch (m_Mode) {
	case Mode::FadeOutOnly:
		m_Phase = Phase::FadeOut;
		m_Alpha = 0.0f;
		break;
	case Mode::FadeInOnly:
		// ここで切り替えると遷移が二回行われるので、updateで切り替える
		//m_pSceneManager->ChangeScene(m_nextScene); // 先にシーン変更
		m_Phase = Phase::FadeIn;
		m_Alpha = 1.0f;
		break;
	case Mode::FadeInOut:
		m_Phase = Phase::FadeOut;
		m_Alpha = 0.0f;
		break;
	}
}

/**
 * @brief フェードの進行を更新する
 *
 * 経過時間に基づいて透明度（m_Alpha）を変化させる。
 * フェードアウト終了後はシーンを切り替え、フェードインに移行する。
 *
 * @param deltaTime 前フレームからの経過時間（マイクロ秒）
 */
void FadeTransition::Update(const float deltaTime)
{
	// フェーズが None なら何もしない
	if (m_Phase == Phase::None) { return; }

	// 単純にこのフェーズに対する経過時間だけを見る
	m_Elapsed += deltaTime;

	switch (m_Phase) {
	case Phase::FadeOut:
	{
		// フェードアウト進行
		float t = std::min(m_Elapsed / m_Duration, 1.0f);
		m_Alpha = t;
		if (t >= 1.0f)
		{
			// フェードアウト完了
			if (m_Mode == Mode::FadeOutOnly)
			{
				// ここで終わり（黒のまま切り替え終了）
				m_Phase = Phase::None;
			}
			else if (m_Mode == Mode::FadeInOut)
			{
				// SceneManager に「今が切り替えタイミング」と伝えたい
				m_Phase = Phase::WaitSceneChange;
				// 次のフェーズに備えてリセット
				m_Elapsed = 0.0f;
			}
		}
		break;
	}

	case Phase::WaitSceneChange:
		// SceneManager が ChangeSceneImmediate を呼ぶまでここで待機
		// alpha は 1.0（真っ黒）のまま
		m_Alpha = 1.0f;
		break;

	case Phase::FadeIn:
		// FadeInOnlyの場合はここでシーン切り替えを行う
	{
		float t = std::min(m_Elapsed / m_Duration, 1.0f);
		m_Alpha = 1.0f - t;

		if (t >= 1.0f)
		{
			m_Alpha = 0.0f;
			m_Phase = Phase::None;
		}
		break;
	}

	case Phase::None:
		// 何もしない
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
void FadeTransition::Draw(void)
{
	if (m_Phase != Phase::None)
	{
		//BoxDrawerDraw(
		//	SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		//	Color(0, 0, 0, m_Alpha),
		//	SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0
		//);
		m_Box.Draw(
			Matrix4x4::CreateTranslation(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0.0f),
			Color(0, 0, 0, m_Alpha)
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
bool FadeTransition::IsFinished(void) const
{
	return m_Phase == Phase::None;
}

bool FadeTransition::NeedsSceneChange(void) const
{
	// フェードアウト完了後、黒画面になって SceneManager に切り替えを依頼したい瞬間
	return m_Phase == Phase::WaitSceneChange;
}



void FadeTransition::OnSceneChanged(void)
{
	// SceneManager が ChangeSceneImmediate を呼んだ直後にこれを呼ぶ
	if (m_Mode == Mode::FadeInOut || m_Mode == Mode::FadeInOnly)
	{
		m_Phase = Phase::FadeIn;
		m_Elapsed = 0.0f;
		// alpha は 1.0（真っ黒）から 0.0 へ落としていく
		m_Alpha = 1.0f;
	}
	else
	{
		// FadeOutOnly の場合はもう終わりで良い
		m_Phase = Phase::None;
	}
}