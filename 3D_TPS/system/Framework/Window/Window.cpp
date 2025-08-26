#include "Window.h"


/**
 * @brief ウィンドウの初期化
 *
 * ウィンドウの登録と作成"だけ"を行う
 *
 * @param ウィンドウの幅、高さ
*/
bool Window::Init(uint32_t _Screen_width, uint32_t _Screen_height) {
	//! 引数で設定したウィンドウサイズをメンバ変数に代入
	m_Width = _Screen_width;
	m_Height = _Screen_height;

	//! ウィンドウの初期化
	// インスタンスハンドルを取得.
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}

	// ウィンドウの設定.
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

	// ウィンドウの登録.
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// インスタンスハンドル設定.
	m_hInst = hInst;

	// ウィンドウのサイズを設定.
	RECT rc = {};
	rc.right = static_cast<LONG>(m_Width);
	rc.bottom = static_cast<LONG>(m_Height);

	// ウィンドウサイズを調整.
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// ウィンドウを生成.
	m_hWnd = CreateWindowEx(
		0,
		//        WS_EX_TOPMOST,
		ClassName,				// 拡張ウィンドウスタイル
		WindowName,				// ウィンドウの名前
		style,					// ウィンドウスタイル
		CW_USEDEFAULT,			// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,			// ウィンドウの左上Ｙ座標 
		rc.right - rc.left,		// ウィンドウの幅
		rc.bottom - rc.top,		// ウィンドウの高さ
		nullptr,				// 親ウィンドウのハンドル
		nullptr,				// メニューハンドルまたは子ウィンドウID
		m_hInst,				// インスタンスハンドル
		nullptr);				// ウィンドウ作成データ

	// もしウィンドウの情報がない（ウィンドウ作成がうまくいってない）なら
	if (m_hWnd == nullptr)
	{
		// 初期化失敗とする
		return false;
	}

	// ウィンドウを表示.
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// ウィンドウを更新.
	UpdateWindow(m_hWnd);

	// ウィンドウにフォーカスを設定.
	SetFocus(m_hWnd);

	// 正常終了（ウィンドウハンドルを返す）.
	return true;
}


//-----------------------------------------------------------------------------
// メッセージループ（メッセージがある間だけループする関数）
//-----------------------------------------------------------------------------
bool Window::MessageLoop(void) {
	// 新たにメッセージ(WM_KEYDOWNとかそういうやつ)があれば
	while (PeekMessage(&m_Msg, NULL, 0, 0, PM_REMOVE))
	{
		// ウィンドウプロシージャ関数にメッセージを送る
		TranslateMessage(&m_Msg);
		DispatchMessage(&m_Msg);

		// 「WM_QUIT」メッセージを受け取ったらループを抜ける
		if (m_Msg.message == WM_QUIT) {
			// ゲーム終了（ウィンドウを閉じる）
			return false;
		}
	}
	//! メッセージがない(なくなった)ので次の処理へ
	return true;
}


//-----------------------------------------------------------------------------
// ウィンドウの終了処理
//-----------------------------------------------------------------------------
void Window::Uninit(void) {
	// ウィンドウの登録を解除.
	if (m_hInst != nullptr)
	{
		UnregisterClass(ClassName, m_hInst);
	}

	m_hInst = nullptr;
	m_hWnd = nullptr;
}


//--------------------------------------------------------------------------------------
//ウィンドウプロシージャ（プロシージャ = 手続き）
//--------------------------------------------------------------------------------------
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 状態管理用変数
	static bool isFullScreen = false;
	static bool isMessageBoxShowed = false;

	switch (uMsg)
	{
	case WM_DESTROY:// ウィンドウ破棄のメッセージ
		PostQuitMessage(0);// 「WM_QUIT」メッセージを送る　→　アプリ終了
		break;
	case WM_CLOSE:  // 「x」ボタンが押されたら
	{
		// メッセージボック表示中にも同時実行されるのでここに入る可能性がある
		isMessageBoxShowed = true;
		int res = MessageBoxA(NULL, "終了しますか？", "確認", MB_OKCANCEL);
		isMessageBoxShowed = false;
		if (res == IDOK) {
			DestroyWindow(hWnd);  // 「WM_DESTROY」メッセージを送る
		}
	}
	break;
	case WM_KEYDOWN: //キー入力があった場合のメッセージ
		if (LOWORD(wParam) == VK_ESCAPE) { //入力されたキーがESCAPEなら
			PostMessage(hWnd, WM_CLOSE, wParam, lParam);//「WM_CLOSE」を送る
		}
		else if (LOWORD(wParam) == VK_F11) {
			// F11キーでフルスクリーンに切り替え
			isFullScreen = !isFullScreen;
			if (isFullScreen) {
				// 疑似フルスクリーンモードに変更
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_MINIMIZEBOX);	// ウィンドウ枠を削除
				// ディスプレイ解像度を取得
				int	screenWidth = GetSystemMetrics(SM_CXSCREEN);
				int screenHeight = GetSystemMetrics(SM_CYSCREEN);
				// ウィンドウサイズを変える
				SetWindowPos(hWnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
			}
			else {
				// 通常ウィンドウに戻す
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);			// ウィンドウ枠を削除
				SetWindowPos(hWnd, HWND_TOP, 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
			}
		}
		else {

		}
		break;
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE) {
			// フルスクリーン表示かつメッセージボックス非表示のとき
			if (isFullScreen && !isMessageBoxShowed) {
				// ウィンドウを最小化する（タスク切り替え時に背後に残る問題対策）
				ShowWindow(hWnd, SW_MINIMIZE);
			}
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	// 標準挙動を実行させるため
		break;
	default:
		// 特別じゃない（↑のcaseで定義されていないような）メッセージ（処理）の場合はこっちに飛ぶ
		// 受け取ったメッセージに対してデフォルトの処理を実行
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

/**
 * @brief ウィンドウハンドル取得関数
 * ウィンドウハンドルはウィンドウの初期化処理内でウィンドウの登録などを済ませてから初めて情報として成立する
 * →staticにする（インスタンス生成前に呼び出せるようにする）必要はない
 * @return ウィンドウハンドル
*/
HWND Window::GetHandleWindow(void)
{
	return this->m_hWnd;
}

