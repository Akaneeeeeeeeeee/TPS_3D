#include "IRenderer.h"
#include "system/Framework/Game/Game.h"

void IRenderer::Attach(EngineServices& context)
{
	// RenderManagerに登録
	if (!m_pRenderManager)
	{
		m_pRenderManager = &context.render;
		m_pRenderManager->Register(this);
	}
}

void IRenderer::Detach(void)
{
	// RenderManagerから解除
	if (m_pRenderManager)
	{
		m_pRenderManager->Unregister(this);
		m_pRenderManager = nullptr;
	}
}