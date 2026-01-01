#pragma once
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "system/Framework/Graphics/RenderManager.h"
#include "system/Framework/Graphics/GraphicsDevice.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/CameraManager/CameraManager.h"
#include "Framework/Component/Camera/CameraComponent.h"
#include "Framework/LightSystem/LightSystem.h"


/*
* @brief	エンジンコンテキスト
* @detail	各種マネージャークラスの参照を保持する構造体
* @remark	ゲームオブジェクト／コンポーネントが使う下位サービスのマネージャ系のみを管理。
* @remarks	シーンマネージャ、オブジェクトマネージャ、コンポーネントマネージャなど、各オブジェクト/コンポーネントから触られたくないような上位サービスは含めない。
* @auther	赤根和樹
* @date		2025/10/02
*/
struct EngineServices
{
	RenderManager& render;
	ShaderManager& shader;
	AssetManager& asset;
	PhysicsManager& physics;
	WeatherSystem& weather;
	CameraManager& camera;
	LightSystem& light;
	//SoundSystem& sound;
};


class EngineSystems
{
public:
	EngineSystems() = default;
	EngineSystems(const EngineSystems&) = delete;
	EngineSystems& operator=(const EngineSystems&) = delete;

	void Init();					// 低レベル初期化
	void BeginFrame(float dt);		// 入力/オーディオ BeginFrame 相当を置く場所
	void UpdateFrame(float dt);		// 物理/天候/ライトGPU更新など「エンジン共通更新」
	void EndFrame();				// 必要なら
	void Uninit();

	EngineServices& GetServices() { return m_Services; }
	const EngineServices& GetServices() const { return m_Services; }

private:
	GraphicsDevice m_Graphics;

	ShaderManager  m_Shader;
	AssetManager   m_Asset;
	RenderManager  m_Render;
	CameraManager  m_Camera;
	WeatherSystem  m_Weather;
	PhysicsManager m_Physics;
	LightSystem    m_Light;
	//SoundSystem    m_Sound;

	EngineServices m_Services{
		m_Render, m_Shader, m_Asset.GetInstance(),
		m_Physics, m_Weather, m_Camera, m_Light, //m_Sound
	};

	bool m_Inited = false;
};