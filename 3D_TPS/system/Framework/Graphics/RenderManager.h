#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/Graphics/RenderInfo.h"

class IRenderer;
class GraphicsDevice;
class ShaderManager;

/// <summary>
/// IRenderComponentを管理し描画を担当するクラス
/// ドローコールの発行とIA(Input Assembler)とのやり取りまでを担当する
/// InputLayoutの設定はVertexShaderに依存するためVSに任せる
/// </summary>
class RenderManager
{
public:
	RenderManager();
	~RenderManager();

	bool Init(GraphicsDevice* graphicsDevice, ShaderManager* shaderMgr);	//! 初期化処理
	void Uninit(void);			//! 終了処理

	void StartRender(void);		//! 描画開始処理
	void Render(const RenderInfo& info);	//! 描画コンポーネント1つ分の描画(これをinfoコンテナ数分ループさせる)
	void RenderAll(void);		//! 登録されている全ての描画コンポーネントを描画
	void EndRender(void);		//! 描画終了処理

	GraphicsDevice* GetGraphicsDevice(void) const;
	ShaderManager* GetShaderManager(void) const;

	void CollectRenderInfo(void);	//! 登録されている全ての描画コンポーネントから描画情報を収集

	// 描画コンポーネントの登録・解除
	void Register(IRenderer* component);
	void Unregister(IRenderer* component);

private:
	GraphicsDevice* m_pGraphicsDevice = nullptr;	//! GraphicsDeviceへのポインタ
	ShaderManager* m_pShaderManager = nullptr;		//! シェーダーマネージャーへのポインタ
	std::vector<IRenderer*> m_RenderComponents;		//! レンダラー系コンポーネントのリスト
	std::vector<RenderInfo> m_RenderInfos;			//! 描画情報のリスト(毎フレーム取得)
};