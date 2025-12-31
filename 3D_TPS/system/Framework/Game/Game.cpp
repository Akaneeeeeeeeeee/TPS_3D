#include "Game.h"
#include "system/Framework/Window/Window.h"
#include "system/renderer.h"
#include "system/DebugUI.h"
#include "system/CDirectInput.h"
#include "fpscontrol.h"
#include "system/Framework/SoundManager/SoundManager.h"
#include "system/Sound/SoundWaveVisualizer.h"

/**
* @brief
* @param
*/
void Game::Init()
{
	Renderer::Init();

	CDirectInput::GetInstance().Init(
		Window::GetInstance().GetHandleInstance(),
		Window::GetInstance().GetHandleWindow(),
		Window::GetInstance().GetWidth(),
		Window::GetInstance().GetHeight()
	);

	// エンジン共通初期化
	m_Engine.Init();
	auto& svc = m_Engine.GetServices();
	
	// ゲーム固有のシステム初期化
	m_GameFeatures.Init(svc);

	// 初期天候
	svc.weather.SetWeather(WeatherType::HeavyRain, 0.0f);

	// ComponentFactory は EngineServices を注入
	m_ComponentFactory.Init(&svc);

	// 既存の ObjectFactory / ObjectManager 初期化
	m_ObjectFactory.Init(&m_ComponentFactory);
	m_ObjectManager.Init(&m_ObjectFactory);

	// シーン開始
	m_SceneManager.Init(&m_ObjectManager, "CollisionTestScene");

	// 既存の独立物（統一したいなら EngineSystems 側に寄せる）
	SoundWaveVisualizer::GetInstance().SetWeatherSystem(&svc.weather);

#ifdef _DEBUG
	DebugUI::Init(Renderer::GetDevice(), Renderer::GetDeviceContext());
#endif
}

/**
 * @brief ゲームのループ処理
 * 主なゲーム処理はここに書く
*/
void Game::Update(const float deltatime)
{
	SoundManager::GetInstance().BeginFrame();

	m_Engine.BeginFrame(deltatime);   // ここで入力更新
	m_Engine.UpdateFrame(deltatime);  // 物理/天候/ライトなど

	// ゲーム終了フラグが立っていない場合
	if (!m_SceneManager.GetIsQuit())
	{
		// イベント発生まではループし続ける
		m_SceneManager.Update(deltatime);
		m_GameFeatures.Update(deltatime);
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

	m_GameFeatures.DrawWorld();

	// todo:ここは後から描画機能に責任を持たせる
	SoundWaveVisualizer::GetInstance().DrawWorld();

	auto& svc = m_Engine.GetServices();

	// デバッグ用当たり判定描画
	//svc.physics.DebugDraw();

	svc.weather.DebugDrawParticles();
	svc.weather.DebugDrawSun();

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
	m_Engine.Uninit();
}

