#include "Entry/main.h"
#include "Application.h"
#include "fpscontrol.h"

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
		uint64_t deltatime = 0;

		// 前回実行されてからの経過時間を計算する
		deltatime = fpsrate.CalcDelta();

		//std::cout << deltatime << std::endl;

		// 更新・描画
		m_Game.Update(deltatime);
		m_Game.Draw();
		// 規定時間までWAIT
		fpsrate.Tick();
	}
	//! 終了処理
	Uninit();
}

void Application::Uninit(void)
{
	Window::GetInstance().Uninit();	//! ウィンドウを終了
	m_Game.Uninit();				//! ゲームを終了
}

