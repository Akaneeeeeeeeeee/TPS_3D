#pragma once
#include "../../IComponent/IComponent.h"
#include "system/Framework/Graphics/RenderManager.h"
#include "system/Framework/ShaderManager/ShaderManager.h"
#include <array>

/**
 * @brief レンダラー系コンポーネントを識別するためのインターフェースクラス
*/
class IRenderer : public IComponent
{
public:
	virtual ~IRenderer() {};

	virtual void Render(void) = 0;

	virtual const std::string& GetShaderName(ShaderStage stage) const {
		return m_ShaderNames[static_cast<size_t>(stage)];
	}

protected:
	IRenderer() : IComponent() {};	//!< コンストラクタ（RenderManagerのポインタはnullptrで初期化）
	IRenderer(RenderManager* renderMgrr) : IComponent() {};

	RenderManager* m_pRenderManager = nullptr;	//!< レンダーマネージャーへのポインタ

	std::array<std::string, static_cast<size_t>(ShaderStage::Stage_MAX)> m_ShaderNames;
};


