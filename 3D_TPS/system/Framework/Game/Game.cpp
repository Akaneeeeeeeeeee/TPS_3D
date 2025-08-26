#include "Game.h"
//#include "system/Framework/Input/Input.h"
#include "system/Framework/Window/Window.h"

/**
 * @brief
 * @param
*/
void Game::Init(void)
{
	// シーンマネージャ、サウンドの初期化
	//Sound::GetInstance().Init();
	m_GraphicsDevice.Init();
	m_SceneManager.SetObjectManager(&m_ObjectManager);
	//m_RenderManager.Init(&m_GraphicsDevice, &m_ShaderManager);
	RenderManager::GetInstance().Init(&m_GraphicsDevice, &m_ShaderManager);
	m_ComponentFactory.Init(&m_ShaderManager);
	//m_ComponentFactory.Init(&m_RenderManager, &m_ShaderManager);
	m_ObjectManager.Init(&m_ComponentFactory);
	m_ShaderManager.Init();
	m_SceneManager.Init();
}


/**
 * @brief ゲームのループ処理
 * 主なゲーム処理はここに書く
*/
void Game::Update(void)
{
	// ゲーム終了フラグが立っていない場合
	if (!m_SceneManager.GetIsQuit())
	{
		// イベント発生まではループし続ける
		m_SceneManager.Update();
	}
	// ゲーム終了フラグが立ったら
	else
	{
		//「終了しますか？」を表示して
		PostMessage(Window::GetInstance().GetHandleWindow(), WM_CLOSE, 0, 0);
		// ゲーム終了フラグをリセット
		m_SceneManager.SetIsQuit(false);
	}
}

void Game::Draw(void)
{
	m_SceneManager.Draw();
}

void Game::Uninit(void)
{
	m_SceneManager.Uninit();
	//Sound::GetInstance().Uninit();
}

