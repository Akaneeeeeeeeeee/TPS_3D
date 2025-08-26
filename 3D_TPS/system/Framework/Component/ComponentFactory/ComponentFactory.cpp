#include "ComponentFactory.h"


ComponentFactory::ComponentFactory()
{
}


ComponentFactory::~ComponentFactory()
{
}


void ComponentFactory::Init(RenderManager* renderManager, ShaderManager* shaderManager)
{
	m_pRenderManager = renderManager;
	m_pShaderManager = shaderManager;
	// 初期化処理が必要な場合はここに追加
}

void ComponentFactory::Init(ShaderManager* shaderManager)
{
	m_pShaderManager = shaderManager;
	// 初期化処理が必要な場合はここに追加
}