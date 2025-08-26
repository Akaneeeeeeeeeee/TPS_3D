#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/NonCopyable/Singleton_Template.h"
#include "system/Framework/Graphics/GraphicsDevice.h"
#include "system/Framework/ShaderManager/ShaderManager.h"

class IRenderer;

/// <summary>
/// IRenderComponentを管理し描画を担当するクラス
/// </summary>
class RenderManager : public Singleton<RenderManager>
{
public:
	RenderManager() {};
	~RenderManager() {};

	bool Init(GraphicsDevice* graphicsDevice, ShaderManager* shaderMgr);	//! 初期化処理
	void Uninit(void);			//! 終了処理

	void StartRender(void);		//! 描画開始処理
	void RenderAll(void);		//! 登録されている全ての描画コンポーネントを描画
	void EndRender(void);		//! 描画終了処理

	GraphicsDevice* GetGraphicsDevice(void) const { return m_pGraphicsDevice; }	//! GraphicsDeviceの取得
	ShaderManager* GetShaderManager(void) const { return m_pShaderManager; }	//! シェーダーマネージャーの取得

	// 描画コンポーネントの登録・解除
	void RegisterRenderComponent(IRenderer* component);
	void UnregisterRenderComponent(IRenderer* component);

private:
	GraphicsDevice* m_pGraphicsDevice = nullptr;	//! GraphicsDeviceへのポインタ
	ShaderManager* m_pShaderManager = nullptr;		//! シェーダーマネージャーへのポインタ
	std::vector<IRenderer*> m_RenderComponents;		//! レンダラー系コンポーネントのリスト
};

