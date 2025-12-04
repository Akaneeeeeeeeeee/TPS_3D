#pragma once
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "system/Framework/Graphics/RenderManager.h"
//#include "system/Framework/ColliderManager/ColliderManager.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/WeatherSystem/WeatherSystem.h"
#include "Framework/CameraManager/CameraManager.h"


/*
* @brief	エンジンコンテキスト
* @detail	各種マネージャークラスの参照を保持する構造体
* @remark	ゲームオブジェクト／コンポーネントが使う下位サービスのマネージャ系のみを管理。
* @remarks	シーンマネージャ、オブジェクトマネージャ、コンポーネントマネージャなど、各オブジェクト/コンポーネントから触られたくないような上位サービスは含めない。
* @auther	赤根和樹
* @date		2025/10/02
*/
struct EngineContext
{
	RenderManager& renderManager;
	ShaderManager& shaderManager;
	AssetManager& assetManager;
	//ColliderManager& colliderManager;
	PhysicsManager& joltPhysicsManager;
	WeatherSystem& weatherSystem;
	CameraManager& cameraManager;
	
	void Update(const float deltaTime) {
		joltPhysicsManager.Update(deltaTime);
	}

	EngineContext(
		RenderManager& rm,
		ShaderManager& sm,
		AssetManager& am,
		PhysicsManager& pm,
		WeatherSystem& ws,
		CameraManager& cm)
		: renderManager(rm), 
		shaderManager(sm), 
		assetManager(am), 
		joltPhysicsManager(pm),
		weatherSystem(ws),
		cameraManager(cm)
	{
	}
};