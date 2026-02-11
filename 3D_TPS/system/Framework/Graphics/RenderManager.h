#pragma once
#include "system/Framework/Application/Entry/main.h"
#include "system/Framework/Graphics/RenderInfo.h"
#include "system/Framework/Graphics/GBuffer.h"
#include "system/Framework/ShadowMap/ShadowMap.h"
#include "system/Framework/ShadowMap/SpotShadowMapArray.h"

class IRenderer;
class GraphicsDevice;
class ShaderManager;
class CShader;
class LightSystem;
class SpotLightGPU;
class ComputeShader;
class LightSystem;
class WeatherSystem;

namespace {
	static constexpr int SPOT_SHADOW_K = 16; // スポットシャドウマップの最大数
	static constexpr int TILE_SIZE = 16;
	static constexpr int TILE_W = SCREEN_WIDTH  / TILE_SIZE; // 120
	static constexpr int TILE_H = SCREEN_HEIGHT / TILE_SIZE; // 68
	static constexpr int TILE_COUNT = TILE_W * TILE_H; // 8160
	static constexpr int MAX_LIGHTS_PER_TILE = 64;

	static constexpr int BEAM_DIV = 2;                 // 1/2 解像度（重いなら 4 に）
	static constexpr int BEAM_W = SCREEN_WIDTH / BEAM_DIV;
	static constexpr int BEAM_H = SCREEN_HEIGHT / BEAM_DIV;
}

struct CBDeferred
{
	Matrix4x4 InvViewT;
	Matrix4x4 InvProjT;
	Vector4   CameraWorldPos; // xyz
	Vector4   Screen;         // xy
};

struct CBTile
{
	Matrix4x4 ViewT;      // Transpose済み（row-vector運用）
	Vector2   Screen;     // (1920,1080)
	Vector2   ProjScale;  // (proj._11, proj._22)
	uint32_t  SpotCount;
	uint32_t  MaxPerTile;
	uint32_t  _pad[2];
};

struct CBTileInfo
{
	uint32_t SpotCount;
	uint32_t MaxPerTile;
	uint32_t TileW;
	uint32_t TileH;
};

struct CBBeam
{
	float beamMaxDist;
	float stepLenWanted;
	float kBeam;
	float beamTint;
	Vector2 BeamSize;
	uint32_t MaxSteps;
	uint32_t _pad;
};

struct CBSpotShadowCPU
{
	Matrix4x4 SpotLightViewProjT[SPOT_SHADOW_K];
	Vector4   SpotShadowTexel;   // (1/w, 1/h, w, h)
	Vector4   SpotShadowParams;  // (bias, normalBias, pcfRadius, K)
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

	bool Init(GraphicsDevice* graphicsDevice, LightSystem* light, WeatherSystem* weather);	//! 初期化処理
	void Uninit(void);			//! 終了処理
	void Release(void);		//! 解放処理

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

	// スポットライトのシャドウマップ割り当て
	void AssignSpotShadowSlices(LightSystem& ls, const Vector3& refPos,
		std::vector<SpotLightGPU>& outLights,
		std::array<int, SPOT_SHADOW_K>& outShadowSrcIndex,
		int& outShadowCount);

	bool CreateSpotStructuredBuffer(ID3D11Device* dev, int capacity);
	void UpdateSpotStructuredBuffer(ID3D11DeviceContext* ctx, const std::vector<SpotLightGPU>& lights);

	bool CreateStructuredUAVBuffer(ID3D11Device* dev, UINT numElements,
		ComPtr<ID3D11Buffer>& outBuf,
		ComPtr<ID3D11UnorderedAccessView>& outUAV,
		ComPtr<ID3D11ShaderResourceView>& outSRV);

	bool CreateSpotAccum(ID3D11Device* dev);
	void RunSpotCompute(const CBDeferred& cbDeferred, const Matrix4x4& viewT, float proj11, float proj22);

	bool CreateBeamTex(ID3D11Device* dev);
	void RunBeamCompute(const CBDeferred& cbDeferred, const CBTileInfo& ti);

	// スポットシャドウマップ関連
	void BuildSpotShadowMatrices(const SpotLightGPU& s, Matrix4x4& outView, Matrix4x4& outProj);
	void RenderSpotShadowPass(const std::vector<SpotLightGPU>& lights, const std::array<int, SPOT_SHADOW_K>& shadowIdx, int shadowCount);
private:
	void RenderGBufferPass(void);
	void RenderLightingPass(void);
	void RenderTransparentForwardPass(void);
	void RenderShadowPass(void);
	void BuildSunShadowMatrices(Matrix4x4& outView, Matrix4x4& outProj) const;

	// 共通のメッシュ描画
	void DrawMeshForward(const MeshDraw& md);
	void DrawMeshGBuffer(const MeshDraw& md);

private:
	GraphicsDevice* m_pGraphicsDevice = nullptr;	//! GraphicsDeviceへのポインタ
	LightSystem* m_pLightSystem = nullptr;			//! LightSystemへのポインタ
	WeatherSystem* m_pWeatherSystem = nullptr;		//! WeatherSystemへのポインタ
	std::vector<IRenderer*> m_RenderComponents;		//! レンダラー系コンポーネントのリスト
	std::vector<RenderPacket> m_Packets;			//! 描画情報のリスト(毎フレーム取得)

	// deferred
	GBuffer m_GBuffer;

	// GBuffer用の通常メッシュシェーダ（mainエントリ）
	CShader* m_pGBufferStatic = nullptr;
	CShader* m_pGBufferSkin = nullptr;

	CShader* m_pDeferredLighting = nullptr;
	ComPtr<ID3D11Buffer>       m_CBDeferred;

	// shadow
	ShadowMap m_Shadow;
	CShader* m_pShadowStatic = nullptr;
	CShader* m_pShadowSkin = nullptr;
	ComputeShader* m_pCSBeam = nullptr;

	ComPtr<ID3D11Buffer> m_CBShadow; // b10
	ComPtr<ID3D11SamplerState> m_ShadowCmpSampler;
	Matrix4x4 m_LightView{};
	Matrix4x4 m_LightProj{};
	Matrix4x4 m_LightViewProjT{};

	// SpotLights StructuredBuffer（SRV）
	ComPtr<ID3D11Buffer> m_SpotSB;
	ComPtr<ID3D11ShaderResourceView> m_SpotSB_SRV;
	int m_SpotSB_Capacity = 0;
	int m_SpotCountThisFrame = 0;

	ComPtr<ID3D11Buffer> m_TileCountBuf;
	ComPtr<ID3D11UnorderedAccessView> m_TileCountUAV;
	ComPtr<ID3D11ShaderResourceView>  m_TileCountSRV;

	ComPtr<ID3D11Buffer> m_TileIndexBuf;
	ComPtr<ID3D11UnorderedAccessView> m_TileIndexUAV;
	ComPtr<ID3D11ShaderResourceView>  m_TileIndexSRV;

	ComPtr<ID3D11Texture2D> m_SpotAccumTex;
	ComPtr<ID3D11UnorderedAccessView> m_SpotAccumUAV;
	ComPtr<ID3D11ShaderResourceView>  m_SpotAccumSRV;

	ComPtr<ID3D11Texture2D> m_BeamTex;
	ComPtr<ID3D11UnorderedAccessView> m_BeamUAV;
	ComPtr<ID3D11ShaderResourceView>  m_BeamSRV;

	ComputeShader* m_pCSBuildTile = nullptr;
	ComputeShader* m_pCSSpotLighting = nullptr;
	// Compute用CB
	ComPtr<ID3D11Buffer> m_CBTile;		// b0 in BuildTileCS
	ComPtr<ID3D11Buffer> m_CBTileInfo;	// b1 in SpotLightingCS
	ComPtr<ID3D11Buffer> m_CBBeam;		// b0 for BeamCS

	SpotShadowMapArray m_SpotShadow;
	CBSpotShadowCPU m_SpotShadowCB;
	ComPtr<ID3D11Buffer> m_CBSpotShadow; // b2
};