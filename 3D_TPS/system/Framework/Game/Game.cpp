#include "Game.h"
#include "system/Framework/Window/Window.h"
#include "system/renderer.h"
#include "system/DebugUI.h"
#include "system/CDirectInput.h"
#include "fpscontrol.h"

Game::Game()
{
}

Game::~Game()
{
}

/**
 * @brief
 * @param
*/
void Game::Init(void)
{
	// シーンマネージャ、サウンドの初期化
	//Sound::GetInstance().Init();
	//m_GraphicsDevice.Init();
	//m_SceneManager.SetSceneFactory(&m_SceneFactory);
	//m_RenderManager.Init(&m_GraphicsDevice, &m_ShaderManager);
	//RenderManager::GetInstance().Init(&m_GraphicsDevice, &m_ShaderManager);
	//m_ComponentFactory.Init(&m_ShaderManager);
	//m_ComponentFactory.Init(&m_RenderManager, &m_ShaderManager);
	//m_ObjectManager.Init(&m_ComponentFactory);
	
	
	// レンダラの初期化
	Renderer::Init();

	// DirectInputの初期化
	CDirectInput::GetInstance().Init(Window::GetInstance().GetHandleInstance(),
		Window::GetInstance().GetHandleWindow(),
		Window::GetInstance().GetWidth(),
		Window::GetInstance().GetHeight());
	
	// シェーダー管理クラスの初期化
	//m_ShaderManager.Init();
	ShaderManager::GetInstance().Init();

	// アセット管理クラスの初期化
	AssetManager::GetInstance().Init();
	//m_GraphicsDevice.Init();
	// レンダーマネージャの初期化
	/*m_RenderManager.Init(&m_GraphicsDevice);
	m_pContext = std::make_unique<EngineContext>(
		m_RenderManager,
		ShaderManager::GetInstance(),
		AssetManager::GetInstance());*/

	// オブジェクトマネージャの初期化
	m_ObjectManager.Init(m_pContext.get());

	// シーンマネージャの初期化
	m_SceneManager.Init(&m_ObjectManager);
	
	// デバッグ時のみ、デバッグUIの初期化
#ifdef _DEBUG
	DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());
#endif // _DEBUG
	
}


/**
 * @brief ゲームのループ処理
 * 主なゲーム処理はここに書く
*/
void Game::Update(const uint64_t deltatime)
{
	CDirectInput::GetInstance().GetKeyBuffer();		// キーボードの状態を取得
	CDirectInput::GetInstance().GetMouseState();	// マウスの状態を取得

	// ゲーム終了フラグが立っていない場合
	if (!m_SceneManager.GetIsQuit())
	{
		// イベント発生まではループし続ける
		m_SceneManager.Update(deltatime);
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

void Game::Draw()
{
	// レンダリング前処理
	Renderer::Begin();
	//m_RenderManager.StartRender();

	// シーンマネージャの描画
	m_SceneManager.Draw();
	/*m_RenderManager.CollectRenderInfo();
	m_RenderManager.RenderAll();*/

	// デバッグUIの描画
#ifdef _DEBUG
	DebugUI::Render();
#endif // _DEBUG

	// レンダリング後処理
	//m_RenderManager.EndRender();
	Renderer::End();
}

void Game::Uninit(void)
{
	// デバッグUIの終了処理
#ifdef _DEBUG
	DebugUI::DisposeUI();
#endif // _DEBUG

	// シーンマネージャの終了処理
	m_SceneManager.Uninit();

	// レンダラの終了処理
	//m_RenderManager.Uninit();
	Renderer::Uninit();
}

