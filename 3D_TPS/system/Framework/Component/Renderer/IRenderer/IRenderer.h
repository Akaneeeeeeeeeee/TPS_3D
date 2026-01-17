#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include <d3d11.h>
#include <vector>

class RenderManager;	// 前方宣言
class ShaderManager;	// 前方宣言
struct RenderPacket;	// 前方宣言

/**
 * @brief レンダラー系コンポーネントを識別するためのインターフェースクラス
*/
class IRenderer : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(IRenderer, IComponent)
	virtual ~IRenderer() = default;

	virtual void CollectRenderPackets(std::vector<RenderPacket>& out) = 0;

	virtual void Attach(EngineServices& context) override;
	virtual void Detach(void) override;

protected:
	IRenderer() : IComponent() {};	//!< コンストラクタ（RenderManagerのポインタはnullptrで初期化）

	RenderManager* m_pRenderManager = nullptr;	//!< レンダーマネージャーへのポインタ(描画系のみが依存する)
};


