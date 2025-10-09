#pragma once
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "system/Framework/Graphics/RenderManager.h"
#include "system/Framework/ColliderManager/ColliderManager.h"

struct EngineContext
{
	RenderManager& renderManager;
	ShaderManager& shaderManager;
	AssetManager& assetManager;
	ColliderManager& colliderManager;

	EngineContext(RenderManager& rm, ShaderManager& sm, AssetManager& am, ColliderManager& cm)
		: renderManager(rm), shaderManager(sm), assetManager(am), colliderManager(cm) {
	}
};