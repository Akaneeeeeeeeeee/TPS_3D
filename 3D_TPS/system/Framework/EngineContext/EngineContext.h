#pragma once
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "system/Framework/Graphics/RenderManager.h"
//#include "system/Framework/ColliderManager/ColliderManager.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"


/*
* @brief	エンジンコンテキスト
* @detail	各種マネージャークラスの参照を保持する構造体
* @remark	各種マネージャークラスの参照を保持し、コンポーネントやシステムに依存性を注入するために使用
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
	
	void Update(const float deltaTime) {
		joltPhysicsManager.Update(deltaTime);
	}

	/*EngineContext(RenderManager& rm, ShaderManager& sm, AssetManager& am, ColliderManager& cm, PhysicsManager& pm)
		: renderManager(rm), shaderManager(sm), assetManager(am), colliderManager(cm), joltPhysicsManager(pm) {
	}*/
	EngineContext(RenderManager& rm, ShaderManager& sm, AssetManager& am, PhysicsManager& pm)
		: renderManager(rm), shaderManager(sm), assetManager(am), joltPhysicsManager(pm) {
	}
};