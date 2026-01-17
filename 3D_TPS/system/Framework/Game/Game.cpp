#include "Game.h"
#include "system/Framework/Window/Window.h"
#include "system/renderer.h"
#include "system/DebugUI.h"
#include "system/CDirectInput.h"
#include "fpscontrol.h"
#include "system/Framework/SoundManager/SoundManager.h"
#include "system/Sound/SoundWaveVisualizer.h"
#include "Framework/Time/Time.h"
#include "system/Framework/WeatherSystem/SkyFogPass.h"

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
	svc.weather.SetWeather(WeatherType::Clear, 0.0f);
	svc.physics.SetObjectManager(&m_ObjectManager);

	// ComponentFactory は EngineServices を注入
	m_ComponentFactory.Init(&svc);

	// 既存の ObjectFactory / ObjectManager 初期化
	m_ObjectFactory.Init(&m_ComponentFactory);
	m_ObjectManager.Init(&m_ObjectFactory);

	// シーン開始
	//m_SceneManager.Init(&m_ObjectManager, "AnimatedTitleScene");
	m_SceneManager.Init(&m_ObjectManager, "GameScene");

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

	// まず入力は更新（解除キーを拾う）
	m_Engine.BeginFrame(deltatime);

	// ここでポーズチェック
	// ESC または P で切り替え
	if (CDirectInput::GetInstance().CheckKeyBufferTrigger(DIK_P))
	{
		SetPaused(!m_IsPaused);
	}

	// ポーズ中は「世界」を更新しない
	if (m_IsPaused)
	{
		return;
	}

	// ここから通常の世界更新
	m_Engine.UpdateFrame(deltatime);

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

	auto& svc = m_Engine.GetServices();
	if (auto* cam = svc.camera.GetMain())
	{
		Matrix4x4 view = cam->GetViewMatrix();
		Matrix4x4 proj = cam->GetProjMatrix();
		/*Renderer::SetViewMatrix(&view);
		Renderer::SetProjectionMatrix(&proj);*/
		cam->ApplyCamera();
	}

	// 1) 空（backbufferへ）
	svc.weather.DrawAtmospherePreWorld();

	// シーンの描画
	m_SceneManager.DrawWorld();
	m_GameFeatures.DrawWorld();

	// 天候パーティクル描画
	svc.weather.DrawParticles();

	// デバッグ用当たり判定描画
	//svc.physics.DebugDraw();

	// 2) 遅延描画（GBuffer→Lighting→(透明Forwardは後で)）
	svc.render.RenderDeferred();

	// 3) 霧（backbufferへ合成）
	svc.weather.DrawAtmospherePostWorld();

	// todo:ここは後から描画機能に責任を持たせる
	SoundWaveVisualizer::GetInstance().DrawWorld();


	// 4) UI
	svc.ui.Draw(SCREEN_WIDTH, SCREEN_HEIGHT);
	m_SceneManager.DrawUI();

	// 遷移フェードを最後に描く
	m_SceneManager.DrawTransition();

	// ポーズUIを上に重ねる
	if (m_IsPaused)
	{
		// ImGuiで描くならここ
		//ImGui::Begin("Pause", nullptr,
		//	ImGuiWindowFlags_NoResize |
		//	ImGuiWindowFlags_AlwaysAutoResize);
		//ImGui::Text("PAUSED");
		//ImGui::Text("Press P (or ESC) to resume.");
		//ImGui::End();

		// DirectWriteで描くなら、あなたの描画経路に合わせてここで DrawString
		// (DirectWriteのrender targetがBegin/Endと噛むならUI専用の描画フェーズに寄せる)
	}

	// ポーズ中はシーン切替確定も止めたいならガードする
	if (!m_IsPaused)
	{
		// 描画しきった後に切り替え確定
		m_SceneManager.CommitSceneChange();
	}


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
	// オブジェクトマネージャの終了処理
	m_ObjectManager.Uninit();

	// レンダラの終了処理
	//m_RenderManager.Uninit();
	Renderer::Uninit();
	m_Engine.Uninit();
}

void Game::SetPaused(bool paused)
{
	if (m_IsPaused == paused) return;
	m_IsPaused = paused;

	if (m_IsPaused)
	{
		// 「ゲーム世界」を止める
		m_PrevTimeScale = Time::GetInstance().GetTimeScale();
		Time::GetInstance().SetTimeScale(0.0f);
	}
	else
	{
		Time::GetInstance().SetTimeScale(m_PrevTimeScale);
		// 必須ではないが、安全側に倒すなら入れてOK（遷移/一時停止解除の瞬間のズレ対策）
		Window::GetInstance().RequestTimeReset();
	}
}