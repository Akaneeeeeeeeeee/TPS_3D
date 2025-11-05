#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include <d3d11.h>
#include "Graphics/RenderInfo.h"
#include <vector>

class RenderManager;	// 前方宣言
class ShaderManager;	// 前方宣言

/**
 * @brief レンダラー系コンポーネントを識別するためのインターフェースクラス
*/
class IRenderer : public IComponent
{
public:
	virtual ~IRenderer() = default;

	virtual bool GetRenderInfo(RenderInfo& outInfo) = 0;	//!< 描画に必要な情報を取得する純粋仮想関数

	// サブセット単位で描画情報を列挙する関数
	virtual void EnumerateRenderInfos(std::vector<RenderInfo>& outInfos)
	{
		RenderInfo info;
		if (GetRenderInfo(info))
		{
			outInfos.push_back(info);
		}
	}

	virtual void Attach(EngineContext& context) override;
	virtual void Detach(EngineContext& context) override;

protected:
	IRenderer() : IComponent() {};	//!< コンストラクタ（RenderManagerのポインタはnullptrで初期化）

	RenderManager* m_pRenderManager = nullptr;	//!< レンダーマネージャーへのポインタ(描画系のみが依存する)
};


