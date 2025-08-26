#pragma once
#include "system/Framework/Window/Window.h"
#include "system/noncopyable.h"
#include "system/Framework/Game/Game.h"
#pragma comment (lib,"winmm.lib")

/**
 * @brief ゲームそのものを管理するクラス（Applicationクラス?）
 * 
 * 流れとしては、
 * ①windowの初期化をする
 * ②HWND（ウィンドウの情報持ってるアドレスへのポインタみたいなもの）を使ってコンストラクタでDirectXの初期化をする
 * ③ゲームの中身の初期化をする
 * という流れで進む。普通に考えれば終了処理はその逆の順番なはず
 * 
 * InputとWindowクラスはシングルトンとして設計
*/
class Application : public NonCopyable
{
public:
	//! メンバ変数は宣言した順に初期化される→先にd3d11のコンストラクタを呼び出し、そのあとにgameクラスのコンストラクタを呼ぶ
	Application(const uint32_t& _width, const uint32_t& _height) : m_Game() {
		Window::GetInstance().Init(_width, _height); // ウィンドウの初期化
		this->Init(); // ゲームの初期化
	};
	~Application() {};


	void Init(void);		// 初期化
	void Run(void);			// ループ
	void Uninit(void);		// 終了
private:
	Game m_Game;			// ゲームクラス
};

