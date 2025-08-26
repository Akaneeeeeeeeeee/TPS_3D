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
	// �������������K�v�ȏꍇ�͂����ɒǉ�
}

void ComponentFactory::Init(ShaderManager* shaderManager)
{
	m_pShaderManager = shaderManager;
	// �������������K�v�ȏꍇ�͂����ɒǉ�
}