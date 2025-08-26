#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/NonCopyable/Singleton_Template.h"

/**
 * @brief ゲームクラスを保持するウィンドウクラス
 *
 * 初期化、ゲームループ、終了処理、ウィンドウプロシージャ関数を機能として持つ
*/
class Window : public Singleton<Window>
{
	friend class Singleton<Window>; // こうしないとWindow自身のコンストラクタに辿り着けない
public:
	/**
	 * @brief ウィンドウの初期化処理
	 * →これ自体は外からでもしたいのでpublicでいい
	 * @param screen_width ウィンドウの幅
	 * @param screen_height ウィンドウの高さ
	 * @return 初期化成功or失敗
	*/
	bool Init(uint32_t _Screen_width = SCREEN_WIDTH, uint32_t _Screen_height = SCREEN_HEIGHT);
	bool MessageLoop(void);		// メッセージループ
	void Uninit(void);			// 終了処理

	/**
	 * @brief ゲームループに直接書くことはないが、ウィンドウに起こったイベント（メッセージ）に応じた処理を記述する関数
	 * @param hWnd ウィンドウ情報
	 * @param uMsg メッセージ（イベント：WM_KEYDOWNとか）の情報
	 * @param wParam 32ビットの情報で、メッセージに関連する値（例：キーコードやボタンの状態）を格納する　　らしい
	 * @param lParam 64ビットの情報で、より詳細なデータ（例：座標情報やポインタ）を格納する　　らしい
	 * @return ウィンドウプロシージャ(WndProc)内でメッセージに対する処理結果を返す際に使う。メッセージの処理が成功した場合、0を返すことが一般的　　らしい
	*/
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND GetHandleWindow(void);		//! ウィンドウハンドル取得関数
	//MSG* GetMessage(void)

	uint32_t GetWidth(void) const { return m_Width; }		//! ウィンドウの横幅取得関数
	uint32_t GetHeight(void) const { return m_Height; }	//! ウィンドウの縦幅取得関数

private:
	// 外部からのインスタンス生成を禁止するためにコンストラクタとデストラクタをprivateにする
	Window() = default;
	~Window() = default;

	HINSTANCE	m_hInst;	// インスタンスハンドル(アプリケーションを識別する情報→これはどんな設計でも単一(static)であるべき)
	HWND		m_hWnd;		// ウィンドウハンドル(ウィンドウの情報を持つポインタみたいなもの→今回はウィンドウは一つなので単一(static)とする)
	uint32_t	m_Width;	// ウィンドウの横幅
	uint32_t	m_Height;	// ウィンドウの縦幅
	MSG			m_Msg;		// ウィンドウのイベントを識別するメッセージを保持するための構造体
};
