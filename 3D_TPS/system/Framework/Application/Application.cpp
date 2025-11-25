#include "Entry/main.h"
#include "Application.h"
#include "fpscontrol.h"
#include "Framework/Time/Time.h"

/**
 * @brief アプリ全体としての初期化処理
 * 
 * ウィンドウ初期化→DirectX初期化→ゲーム初期化
 * の流れで進める
*/
void Application::Init(void)
{
	m_Game.Init();					// ゲーム初期化
}

/**
 * @brief アプリケーション実行
 * ここがメインのループとなる
*/
void Application::Run(void)
{
	// フレームの待ち時間を計算する
	static FPS fpsrate(80);

	//! 終了(WM_QUIT)メッセージがない間はループし続ける
	//! →二重ループに見えるが、MessageLoop()はメッセージがWM_QUIT以外の場合trueを返すので大丈夫(それ以外のメッセージはウィンドウプロシージャに送っておしまい)
	while (Window::GetInstance().MessageLoop())
	{
		// FPS制御
		fpsrate.Tick();
		// Tick で確定したマイクロ秒を Time に渡す
		Time::GetInstance().Update(fpsrate.GetDeltaTime());

		m_Game.Update(Time::GetInstance().Deltatime());
		m_Game.Draw();
	}
	//! 終了処理
	Uninit();
}

void Application::Uninit(void)
{
	Window::GetInstance().Uninit();	//! ウィンドウを終了
	m_Game.Uninit();				//! ゲームを終了
}

