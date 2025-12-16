#include "Game.h"
#include "system/Framework/Window/Window.h"
#include "system/renderer.h"
#include "system/DebugUI.h"
#include "system/CDirectInput.h"
#include "fpscontrol.h"
#include "system/Framework/SoundManager/SoundManager.h"
#include "system/Sound/SoundWaveVisualizer.h"

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


	// 低レベルから初期化
	m_GraphicsDevice.Init();
	// シェーダー管理クラスの初期化
	ShaderManager::GetInstance().Init();
	// アセット管理クラスの初期化
	AssetManager::GetInstance().Init();

	//m_CameraManager.Init();


	// レンダーマネージャの初期化
	m_RenderManager.Init(&m_GraphicsDevice);
	// 物理マネージャの初期化
	m_PhysicsManager.Init();
	m_WeatherSystem.Init();


	m_pContext = std::make_unique<EngineContext>(
		m_RenderManager,
		ShaderManager::GetInstance(),
		AssetManager::GetInstance(),
		m_PhysicsManager,
		m_WeatherSystem,
		m_CameraManager);

	m_WeatherSystem.SetWeather(WeatherType::HeavyRain, 0.0f);

	m_ComponentFactory.Init(m_pContext.get());
	m_ObjectFactory.Init(&m_ComponentFactory);

	// オブジェクトマネージャの初期化
	m_ObjectManager.Init(&m_ObjectFactory);

	// シーンマネージャの初期化
	//m_SceneManager.Init(&m_ObjectManager, "AnimatedTitleScene");
	//m_SceneManager.Init(&m_ObjectManager, "TitleScene");
	m_SceneManager.Init(&m_ObjectManager, "CollisionTestScene");

	SoundWaveVisualizer::GetInstance().SetMaxLoudness(1.0f); // 走り足音の loudness に合わせる
	SoundWaveVisualizer::GetInstance().SetWeatherSystem(&m_WeatherSystem);

	// デバッグ時のみ、デバッグUIの初期化
#ifdef _DEBUG
	DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());
#endif // _DEBUG

}


/**
 * @brief ゲームのループ処理
 * 主なゲーム処理はここに書く
*/
void Game::Update(const float deltatime)
{
	SoundManager::GetInstance().BeginFrame();
	m_pContext->Update(deltatime);

	CDirectInput::GetInstance().Update();		// 入力状態を更新

	// ゲーム終了フラグが立っていない場合
	if (!m_SceneManager.GetIsQuit())
	{
		// イベント発生まではループし続ける
		m_SceneManager.Update(deltatime);
		SoundWaveVisualizer::GetInstance().Update(deltatime);
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

	// 物理デバッグ描画
	//m_PhysicsManager.DebugDraw();


	// todo:ここは後から描画機能に責任を持たせる
	SoundWaveVisualizer::GetInstance().DrawWorld();
	m_pContext->weatherSystem.DebugDrawParticles();
	m_pContext->weatherSystem.DebugDrawSun();

	/*m_RenderManager.CollectRenderInfo();
	m_RenderManager.RenderAll();*/

	// デバッグUIの描画
#ifdef _DEBUG
	DebugUI::Render();
#endif // _DEBUG

	// 描画しきった後に切り替え確定
	m_SceneManager.CommitSceneChange();
	
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
	// オブジェクトマネージャの終了処理
	m_ObjectManager.Uninit();

	// レンダラの終了処理
	//m_RenderManager.Uninit();
	Renderer::Uninit();
}

