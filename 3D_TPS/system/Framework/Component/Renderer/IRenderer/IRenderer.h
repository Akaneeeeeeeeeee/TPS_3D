#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include <d3d11.h>

class RenderManager;	// 前方宣言
class ShaderManager;	// 前方宣言
struct RenderInfo;	// 前方宣言

/**
 * @brief レンダラー系コンポーネントを識別するためのインターフェースクラス
*/
class IRenderer : public IComponent
{
public:
	virtual ~IRenderer() {};

	virtual bool GetRenderInfo(RenderInfo& outInfo) = 0;	//!< 描画に必要な情報を取得する純粋仮想関数

	virtual void Attach(EngineContext& context) override;
	virtual void Detach(EngineContext& context) override;

protected:
	IRenderer() : IComponent() {};	//!< コンストラクタ（RenderManagerのポインタはnullptrで初期化）

	RenderManager* m_pRenderManager = nullptr;	//!< レンダーマネージャーへのポインタ(描画系のみが依存する)
};


