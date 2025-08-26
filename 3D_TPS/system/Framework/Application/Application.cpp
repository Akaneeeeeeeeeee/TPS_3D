#include "Entry/main.h"
#include "Application.h"

/**
 * @brief アプリ全体としての初期化処理
 * 
 * ウィンドウ初期化→DirectX初期化→ゲーム初期化
 * の流れで進める
*/
void Application::Init(void)
{
	//HWND hWnd;						// ウィンドウハンドル代入用変数
	//Window::GetInstance().Init();	// ウィンドウ初期化
	//hWnd = Window::GetInstance().GetHandleWindow();		// ウィンドウハンドル取得
	//d3d11.Init(hWnd);				// ウィンドウ情報を使ってDirectXを初期化
	m_Game.Init();					// ゲーム初期化
	
}

/**
 * @brief アプリケーション実行
 * ここがメインのループとなる
*/
void Application::Run(void)
{
	// FPS計測用変数
	int fpsCounter = 0;
	long long oldTick = GetTickCount64(); // 前回計測時の時間
	long long nowTick = oldTick; // 今回計測時の時間

	// FPS固定用変数
	LARGE_INTEGER liWork; // workがつく変数は作業用変数
	long long frequency;// どれくらい細かく時間をカウントできるか
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	// 時間（単位：カウント）取得
	QueryPerformanceCounter(&liWork);
	long long oldCount = liWork.QuadPart;// 前回計測時の時間
	long long nowCount = oldCount;// 今回計測時の時間


	//! 終了(WM_QUIT)メッセージがない間はループし続ける
	//! →二重ループに見えるが、MessageLoop()はメッセージがWM_QUIT以外の場合trueを返すので大丈夫(それ以外のメッセージはウィンドウプロシージャに送っておしまい)
	while (Window::GetInstance().MessageLoop())
	{
		QueryPerformanceCounter(&liWork);// 現在時間を取得
		nowCount = liWork.QuadPart;

		// 1/60秒が経過したか？
		if (nowCount >= oldCount + frequency / 60) {
			// ゲームループ実行
			m_Game.Update();
			m_Game.Draw();
			fpsCounter++; // ゲーム処理を実行したら＋１する
			oldCount = nowCount;
		}
		nowTick = GetTickCount64();		// 現在時間を取得
		// 前回時間から1000ミリ秒が経過したか？
		if (nowTick >= oldTick + 1000)
		{
			// FPS表示
			char str[32];
			wsprintfA(str, "%dFPS", fpsCounter);	// 数値を文字列に変換
			SetWindowTextA(Window::GetInstance().GetHandleWindow(), str);				// ウィンドウタイトルに表示
			// カウンターリセット
			fpsCounter = 0;
			oldTick = nowTick;

		}
	}
	//! 終了処理
	Uninit();
}

//void Application::Draw(void)
//{
//	d3d11.StartRender();	// 描画開始
//	background.Draw();		// 背景を描画
//	player.Draw();			// プレイヤーを描画
//	d3d11.FinishRender();	// 描画終了
//}

void Application::Uninit(void)
{
	//player.Uninit();		// プレイヤーを終了
	//background.Uninit();	// プレイヤーを終了
	Window::GetInstance().Uninit();	// ウィンドウを終了
	m_Game.Uninit();		// ゲームを終了
}

