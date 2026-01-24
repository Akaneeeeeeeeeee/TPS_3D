#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/Graphics/RenderInfo.h"
#include "system/Framework/Graphics/GBuffer.h"

class IRenderer;
class GraphicsDevice;
class ShaderManager;
class CShader;

struct CBDeferred
{
	Matrix4x4 InvViewT;
	Matrix4x4 InvProjT;
	Vector4   CameraWorldPos; // xyz
	Vector4   Screen;         // xy
};

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

	bool Init(GraphicsDevice* graphicsDevice);	//! 初期化処理
	void Uninit(void);			//! 終了処理

	void StartRender(void);		//! 描画開始処理
	//void Render(const RenderInfo& info);	//! 描画コンポーネント1つ分の描画(これをinfoコンテナ数分ループさせる)
	//void RenderAll(void);		//! 登録されている全ての描画コンポーネントを描画
	void RenderDeferred(void);   // GBuffer→Lighting→Forward
	void CollectRenderPackets(void);
	void RenderOverlayWorldPass(void);
	void RenderOverlay2DPass();
	void EndRender(void);		//! 描画終了処理

	// 描画コンポーネントの登録・解除
	void Register(IRenderer* component);
	void Unregister(IRenderer* component);

	void InitDeferredShaders(void);

private:
	void RenderGBufferPass(void);
	void RenderLightingPass(void);
	void RenderTransparentForwardPass(void);

	// 共通のメッシュ描画
	void DrawMeshForward(const MeshDraw& md);
	void DrawMeshGBuffer(const MeshDraw& md);

private:
	GraphicsDevice* m_pGraphicsDevice = nullptr;	//! GraphicsDeviceへのポインタ
	std::vector<IRenderer*> m_RenderComponents;		//! レンダラー系コンポーネントのリスト
	std::vector<RenderPacket> m_Packets;			//! 描画情報のリスト(毎フレーム取得)

	GBuffer m_GBuffer;

	// GBuffer用の通常メッシュシェーダ（mainエントリ）
	CShader* m_pGBufferStatic = nullptr;
	CShader* m_pGBufferSkin = nullptr;

	CShader* m_pDeferredLighting = nullptr;
	ComPtr<ID3D11Buffer>       m_CBDeferred;
};