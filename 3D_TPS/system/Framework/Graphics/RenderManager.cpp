#include "RenderManager.h"
#include "system/Framework/Graphics/GraphicsDevice.h"
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "system/Framework/Window/Window.h"
#include "system/CShader.h"
#include "system/CTexture.h"
#include "system/CMaterial.h"
#include "system/CSprite.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "renderer.h"
#include "BoneCombMatrix.h"
#include "Framework/LightSystem/LightSystem.h"
#include "system/ComputeShader.h"
#include "Framework/WeatherSystem/SkyFogPass.h"

void RenderManager::InitDeferredShaders(void)
{
	auto* dev = Renderer::GetDevice();

	auto& am = AssetManager::GetInstance();
	// Deferred lighting
	m_pDeferredLighting = am.GetShader<CShader>("deferred_lighting");
	if (!m_pDeferredLighting)
		throw std::runtime_error("deferred_lighting shader not found.");

	// GBuffer
	m_pGBufferStatic = am.GetShader<CShader>("gbuffer_static");
	m_pGBufferSkin = am.GetShader<CShader>("gbuffer_skin");
	if (!m_pGBufferStatic || !m_pGBufferSkin)
		throw std::runtime_error("gbuffer shaders not found.");

	// Shadow depth
	m_pShadowStatic = am.GetShader<CShader>("shadow_static");
	m_pShadowSkin = am.GetShader<CShader>("shadow_skin");
	if (!m_pShadowStatic || !m_pShadowSkin)
		throw std::runtime_error("shadow shaders not found.");

	// Compute shaders
	m_pCSBuildTile = am.GetComputeShader("cs_build_tile");
	m_pCSSpotLighting = am.GetComputeShader("cs_spot_lighting");

	if (!m_pCSBuildTile || !m_pCSSpotLighting)
		throw std::runtime_error("compute shaders not found.");

	m_pCSBeam = am.GetComputeShader("cs_beam");
	if (!m_pCSBeam) 
		throw std::runtime_error("cs_beam not found.");

	// 影用：比較サンプラ作成（s1）
	{
		D3D11_SAMPLER_DESC sd{};
		sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

		// 範囲外は「影なし」にしたいので 1
		sd.BorderColor[0] = 1.0f;
		sd.BorderColor[1] = 1.0f;
		sd.BorderColor[2] = 1.0f;
		sd.BorderColor[3] = 1.0f;

		// depth - b <= shadowDepth の判定にしたいので LessEqual
		sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT hr = dev->CreateSamplerState(&sd, m_ShadowCmpSampler.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Create ShadowCmpSampler failed.");
	}

	// CBDeferred（b9）
	{
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(CBDeferred);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		auto* dev = Renderer::GetDevice();
		HRESULT hr = dev->CreateBuffer(&bd, nullptr, m_CBDeferred.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Create CBDeferred failed.");
	}

	// CBShadow（b10）
	{
		struct CBShadow
		{
			Matrix4x4 LightViewProjT;
			Vector4   ShadowTexel;   // (1/w,1/h,w,h)
			Vector4   ShadowParams;  // (bias, normalBias, pcfRadius, 0)
		};

		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(CBShadow);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		auto* dev = Renderer::GetDevice();
		HRESULT hr = dev->CreateBuffer(&bd, nullptr, m_CBShadow.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Create CBShadow failed.");
	}

	// CBTile（b0）
	{
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(CBTile);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		HRESULT hr = dev->CreateBuffer(&bd, nullptr, m_CBTile.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Create CBTile failed.");
	}

	// CBTileInfo（b1）
	{
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(CBTileInfo);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		HRESULT hr = dev->CreateBuffer(&bd, nullptr, m_CBTileInfo.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Create CBTileInfo failed.");
	}

	// CBBeam（b0）
	{
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = sizeof(CBBeam);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		HRESULT hr = dev->CreateBuffer(&bd, nullptr, m_CBBeam.GetAddressOf());
		if (FAILED(hr)) throw std::runtime_error("Create CBBeam failed.");
	}
}

RenderManager::RenderManager()
	: m_pGraphicsDevice(nullptr)
{
}

RenderManager::~RenderManager()
{
	//this->Uninit();
}

// 初期化処理
bool RenderManager::Init(GraphicsDevice* graphicsDevice, LightSystem* light, WeatherSystem* weather)
{
	if (graphicsDevice == nullptr) { return false; }

	m_pLightSystem = light;
	m_pWeatherSystem = weather;

	// GBufferの初期化
	auto* dev = Renderer::GetDevice();
	m_GBuffer.Create(dev, Window::GetInstance().GetWidth(), Window::GetInstance().GetHeight());

	if(!m_Shadow.Create(dev, 2048, 2048))
	{
		return false;
	}

	// Deferred用シェーダーの初期化
	InitDeferredShaders();

	CreateSpotStructuredBuffer(dev, 128); // 100本 + 余裕
	CreateStructuredUAVBuffer(dev, TILE_COUNT, m_TileCountBuf, m_TileCountUAV, m_TileCountSRV);
	CreateStructuredUAVBuffer(dev, TILE_COUNT * MAX_LIGHTS_PER_TILE, m_TileIndexBuf, m_TileIndexUAV, m_TileIndexSRV);
	CreateSpotAccum(dev);
	CreateBeamTex(dev);

	// スポット影配列（K枚）
	m_SpotShadow.Create(dev, 1024, 1024, SPOT_SHADOW_K);
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(CBSpotShadowCPU);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = dev->CreateBuffer(&bd, nullptr, m_CBSpotShadow.GetAddressOf());
	if (FAILED(hr)) throw std::runtime_error("Create CBSpotShadow failed.");

	return true;
}

void RenderManager::Release()
{
	// まずGPUの結びつきを外す（内部参照を落とす）
	if (auto* ctx = Renderer::GetDeviceContext())
	{
		ctx->ClearState();
		ctx->Flush();
	}

	// 他クラスに渡した参照を先に切る
	SkyFogPass::SetBeamSRV(nullptr);

	// 生成したD3Dリソースを全部Reset
	m_ShadowCmpSampler.Reset();

	m_CBDeferred.Reset();
	m_CBShadow.Reset();
	m_CBTile.Reset();
	m_CBTileInfo.Reset();
	m_CBBeam.Reset();
	m_CBSpotShadow.Reset();

	m_SpotSB_SRV.Reset();
	m_SpotSB.Reset();
	m_SpotSB_Capacity = 0;
	m_SpotCountThisFrame = 0;

	m_TileCountBuf.Reset();
	m_TileCountUAV.Reset();
	m_TileCountSRV.Reset();
	m_TileIndexBuf.Reset();
	m_TileIndexUAV.Reset();
	m_TileIndexSRV.Reset();

	m_SpotAccumSRV.Reset();
	m_SpotAccumUAV.Reset();
	m_SpotAccumTex.Reset();

	m_BeamSRV.Reset();
	m_BeamUAV.Reset();
	m_BeamTex.Reset();

	m_GBuffer.Release();
	m_Shadow.Release();
	m_SpotShadow.Release();

	m_pDeferredLighting = nullptr;
	m_pGBufferStatic = nullptr;
	m_pGBufferSkin = nullptr;
	m_pShadowStatic = nullptr;
	m_pShadowSkin = nullptr;
	m_pCSBuildTile = nullptr;
	m_pCSSpotLighting = nullptr;
	m_pCSBeam = nullptr;

	m_RenderComponents.clear();
	m_Packets.clear();
}

void RenderManager::Uninit(void)
{
	this->Release();
	// 依存性の解消
	m_pGraphicsDevice = nullptr;
}

// 描画コンポーネントの登録
void RenderManager::Register(IRenderer* component)
{
	if (!component) { return; }
	this->m_RenderComponents.push_back(component);
}

// 描画コンポーネントの解除
void RenderManager::Unregister(IRenderer* component)
{
	this->m_RenderComponents.erase(
		std::remove(this->m_RenderComponents.begin(), this->m_RenderComponents.end(), component),
		this->m_RenderComponents.end());
}


//////////////////////////////////////////////////
//					描画処理						//
//////////////////////////////////////////////////

// 描画開始処理
void RenderManager::StartRender(void)
{
	//m_pGraphicsDevice->StartRender();
	Renderer::Begin();
}

// 描画情報収集
void RenderManager::CollectRenderPackets()
{
	m_Packets.clear();
	for (auto* c : m_RenderComponents)
	{
		if (!c || !c->GetIsValid()) continue;
		c->CollectRenderPackets(m_Packets);
	}
}

// GBuffer→Lighting→Forward
void RenderManager::RenderDeferred()
{
	CollectRenderPackets();

	// 1) 太陽影
	RenderShadowPass();
	// 2) 不透明を書き溜め
	RenderGBufferPass();

	// ---- ここでスポット用Computeを回す ----
	LightSystem& ls = *m_pLightSystem;

	// 1) ライトキャッシュ更新
	ls.UpdateCache();

	// 2) 近い順でshadowSlice割り当て（今は影マップ未実装なのでsliceだけ付く）
	std::vector<SpotLightGPU> spotLights;
	std::array<int, SPOT_SHADOW_K> shadowIdx{};
	int shadowCount = 0;

	// 参照位置はプレイヤーorカメラ
	Matrix4x4 invView = Renderer::GetViewMatrix().Invert();
	Vector3 refPos = invView.Translation();

	AssignSpotShadowSlices(ls, refPos, spotLights, shadowIdx, shadowCount);

	// ここで近いshadowCount本だけ影マップ生成
	RenderSpotShadowPass(spotLights, shadowIdx, shadowCount);

	// 3) StructuredBuffer更新
	UpdateSpotStructuredBuffer(Renderer::GetDeviceContext(), spotLights);

	// 4) Compute実行
	// CBDeferredはLightingPassと同じ作り方でOK（ここで用意）
	Matrix4x4 view = Renderer::GetViewMatrix();
	Matrix4x4 proj = Renderer::GetProjectionMatrix();
	Matrix4x4 invV = view.Invert();
	Matrix4x4 invP = proj.Invert();

	CBDeferred cbd{};
	cbd.InvViewT = invV.Transpose();
	cbd.InvProjT = invP.Transpose();
	Vector3 camPos = invV.Translation();
	cbd.CameraWorldPos = Vector4(camPos.x, camPos.y, camPos.z, 0);
	const float w = (float)Window::GetInstance().GetWidth();
	const float h = (float)Window::GetInstance().GetHeight();

	cbd.Screen = Vector4(w, h, 0, 0);

	// タイル情報
	CBTileInfo ti{};
	ti.SpotCount = (uint32_t)m_SpotCountThisFrame;
	ti.MaxPerTile = MAX_LIGHTS_PER_TILE;
	ti.TileW = TILE_W;
	ti.TileH = TILE_H;

	// ViewT / ProjScale
	Matrix4x4 viewT = view.Transpose();
	float proj11 = proj._11; // あなたのMatrix4x4の要素名に合わせて
	float proj22 = proj._22;

	RunSpotCompute(cbd, viewT, proj11, proj22);

	RunBeamCompute(cbd, ti);

	// ---- ここまで ----

	RenderLightingPass();
	// 4) 透明
	RenderTransparentForwardPass();
	// 5) オーバーレイ（ワールド空間）
	RenderOverlayWorldPass();
}

void RenderManager::RenderGBufferPass()
{
	auto* ctx = Renderer::GetDeviceContext();

	// RendererのDSVを使ってMRTへ
	m_GBuffer.Begin(ctx, Renderer::GetDSV());

	// GBufferではライト不要、半透明も不要
	Renderer::SetDepthEnable(true);
	Renderer::SetBlendState(BS_NONE);

	// infoを回す
	for (const auto& p : m_Packets)
	{
		if (p.phase != RenderPhase::OpaqueGBuffer) continue;
		if (p.type != DrawType::Mesh) continue;

		const MeshDraw& md = std::get<MeshDraw>(p.payload);
		DrawMeshGBuffer(md);
	}

	// backbufferへ戻す（Clearしない）
	Renderer::BindBackbuffer(true);
}


void RenderManager::RenderLightingPass()
{
	auto* ctx = Renderer::GetDeviceContext();

	ID3D11RenderTargetView* bb = Renderer::GetRTV();
	ctx->OMSetRenderTargets(1, &bb, nullptr);

	D3D11_VIEWPORT vp{};
	vp.Width = (float)Window::GetInstance().GetWidth();
	vp.Height = (float)Window::GetInstance().GetHeight();
	vp.MinDepth = 0; vp.MaxDepth = 1;
	ctx->RSSetViewports(1, &vp);

	ctx->IASetInputLayout(nullptr);
	ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->HSSetShader(nullptr, nullptr, 0);
	ctx->DSSetShader(nullptr, nullptr, 0);

	// Deferred shader
	if (!m_pDeferredLighting) throw std::runtime_error("deferred shader null");
	m_pDeferredLighting->SetGPU();

	// CBDeferred（b9）
	Matrix4x4 view = Renderer::GetViewMatrix();
	Matrix4x4 proj = Renderer::GetProjectionMatrix();
	Matrix4x4 invView = view.Invert();
	Matrix4x4 invProj = proj.Invert();

	CBDeferred cb{};
	cb.InvViewT = invView.Transpose();
	cb.InvProjT = invProj.Transpose();
	Vector3 camPos = invView.Translation();
	cb.CameraWorldPos = Vector4(camPos.x, camPos.y, camPos.z, 0);
	cb.Screen = Vector4((float)vp.Width, (float)vp.Height, 0, 0);

	ctx->UpdateSubresource(m_CBDeferred.Get(), 0, nullptr, &cb, 0, 0);
	ID3D11Buffer* cb9 = m_CBDeferred.Get();
	ctx->VSSetConstantBuffers(9, 1, &cb9);
	ctx->PSSetConstantBuffers(9, 1, &cb9);

	// CBShadow（b10）
	{
		struct CBShadow
		{
			Matrix4x4 LightViewProjT;
			Vector4   ShadowTexel;
			Vector4   ShadowParams;
		} s{};

		s.LightViewProjT = m_LightViewProjT;
		s.ShadowTexel = Vector4(
			1.0f / (float)m_Shadow.GetW(),
			1.0f / (float)m_Shadow.GetH(),
			(float)m_Shadow.GetW(),
			(float)m_Shadow.GetH()
		);

		// bias / normalBias / pcfRadius
		s.ShadowParams = Vector4(
			0.0015f, // bias
			0.0040f, // normalBias
			1.0f,    // pcfRadius（1なら3x3）
			0.0f
		);

		ctx->UpdateSubresource(m_CBShadow.Get(), 0, nullptr, &s, 0, 0);
		ID3D11Buffer* cb10 = m_CBShadow.Get();
		ctx->VSSetConstantBuffers(10, 1, &cb10);
		ctx->PSSetConstantBuffers(10, 1, &cb10);
	}

	// SRV bind（t0..t3 + t4）
	ID3D11ShaderResourceView* srvs[6] = {
		m_GBuffer.GetSRV(0),
		m_GBuffer.GetSRV(1),
		m_GBuffer.GetSRV(2),
		Renderer::GetDepthSRV(),
		m_Shadow.GetSRV(),
		m_SpotAccumSRV.Get()
	};
	ctx->PSSetShaderResources(0, 6, srvs);

	// 影用サンプラを s1 にセット
	ID3D11SamplerState* shadowSamp = m_ShadowCmpSampler.Get();
	ctx->PSSetSamplers(1, 1, &shadowSamp);

	Renderer::SetDepthEnable(false);
	Renderer::SetBlendState(BS_ALPHABLEND);

	ctx->Draw(3, 0);

	Renderer::SetBlendState(BS_NONE);
	Renderer::SetDepthEnable(true);

	// SRV解除
	ID3D11ShaderResourceView* nulls[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ctx->PSSetShaderResources(0, 6, nulls);
}

// 描画終了処理
void RenderManager::EndRender(void)
{
	//m_pGraphicsDevice->FinishRender();
	Renderer::End();
}

void RenderManager::RenderTransparentForwardPass()
{
	auto* ctx = Renderer::GetDeviceContext();

	// backbuffer + DSV に戻す
	Renderer::BindBackbuffer(true);

	// 深度テストON / 深度書き込みOFF
	Renderer::SetDepthReadOnly(true);
	Renderer::SetBlendState(BS_ALPHABLEND);

	// 透明だけ描く
	for (const auto& p : m_Packets)
	{
		if (p.phase != RenderPhase::TransparentForward) continue;
		if (p.type != DrawType::Mesh) continue;

		const MeshDraw& md = std::get<MeshDraw>(p.payload);
		DrawMeshForward(md);
	}

	// 深度テスト/書き込みON、ブレンド無しに戻す
	Renderer::SetDepthEnable(true);
	Renderer::SetBlendState(BS_NONE);
}

void RenderManager::DrawMeshGBuffer(const MeshDraw& md)
{
	if (!md.vb || !md.ib) return;

	auto* ctx = Renderer::GetDeviceContext();

	UINT offset = 0;
	ctx->IASetPrimitiveTopology(md.topology);
	ctx->IASetVertexBuffers(0, 1, &md.vb, &md.stride, &offset);
	ctx->IASetIndexBuffer(md.ib, md.indexFormat, 0);

	// skinned判定（itemsにbonesがあるか）
	bool isSkinned = false;
	for (const auto& di : md.items) { if (di.bones) { isSkinned = true; break; } }
	(isSkinned ? m_pGBufferSkin : m_pGBufferStatic)->SetGPU();

	Matrix4x4 w = md.world;
	Renderer::SetWorldMatrix(&w);

	for (const auto& di : md.items)
	{
		if (di.bones)    di.bones->SetGPU();
		if (di.material) di.material->SetGPU();
		if (di.diffuse)  di.diffuse->SetGPU();
		ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
	}
}

void RenderManager::DrawMeshForward(const MeshDraw& md)
{
	if (!md.vb || !md.ib || !md.shader) return;

	auto* ctx = Renderer::GetDeviceContext();

	// IA
	UINT offset = 0;
	ctx->IASetPrimitiveTopology(md.topology);
	ctx->IASetVertexBuffers(0, 1, &md.vb, &md.stride, &offset);
	ctx->IASetIndexBuffer(md.ib, md.indexFormat, 0);

	// Forward用のシェーダ（通常のVS/PS/GS/レイアウト）
	md.shader->SetGPU();

	// World
	Matrix4x4 w = md.world;
	Renderer::SetWorldMatrix(&w);

	// サブセット描画
	for (const auto& di : md.items)
	{
		if (di.bones)
		{
			// Skin: VS b5
			di.bones->SetGPU();
		}

		if (di.material)
		{
			// Material: b3
			di.material->SetGPU();
		}

		if (di.diffuse)
		{
			// Texture SRV
			di.diffuse->SetGPU();
		}

		ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
	}
}


void RenderManager::RenderOverlayWorldPass()
{
	Renderer::BindBackbuffer(true);
	Renderer::SetBlendState(BS_ALPHABLEND);

	// OverlayWorldだけ抽出してsortKey順
	std::vector<const RenderPacket*> list;
	list.reserve(m_Packets.size());
	for (auto& p : m_Packets)
		if (p.phase == RenderPhase::OverlayWorld) list.push_back(&p);

	std::stable_sort(list.begin(), list.end(),
		[](const RenderPacket* a, const RenderPacket* b) { return a->sortKey < b->sortKey; });

	for (auto* pp : list)
	{
		if (pp->type != DrawType::SpriteBillboard) continue;

		const SpriteDraw& sd = std::get<SpriteDraw>(pp->payload);

		// packetに従う（多くはfalse）
		Renderer::SetDepthEnable(pp->depthTest);

		// sprite側が world セットするなら DrawRaw(world) でOK
		sd.sprite->DrawRaw(sd.world);
	}

	Renderer::SetDepthEnable(true);
	Renderer::SetBlendState(BS_NONE);
}

void RenderManager::RenderOverlay2DPass()
{
	//Renderer::BindBackbuffer(true);

	Renderer::SetDepthEnable(false);
	Renderer::SetBlendState(BS_ALPHABLEND);
	Renderer::DisableCulling(false);

	// UI投影はここで1回だけ
	Renderer::SetWorldViewProjection2D();

	std::vector<const RenderPacket*> list;
	list.reserve(m_Packets.size());
	for (auto& p : m_Packets)
		if (p.phase == RenderPhase::Overlay2D) list.push_back(&p);

	std::stable_sort(list.begin(), list.end(),
		[](const RenderPacket* a, const RenderPacket* b) { return a->sortKey < b->sortKey; });

	for (auto* pp : list)
	{
		if (pp->type != DrawType::Sprite2D) continue;

		const SpriteDraw& sd = std::get<SpriteDraw>(pp->payload);
		sd.sprite->DrawRaw(sd.world);
	}

	Renderer::DisableCulling(true);
	Renderer::SetBlendState(BS_NONE);
	Renderer::SetDepthEnable(true);
}

void RenderManager::RenderShadowPass()
{
	auto* ctx = Renderer::GetDeviceContext();

	// 退避：今のView/Proj（＝カメラ）
	Matrix4x4 savedView = Renderer::GetViewMatrix();
	Matrix4x4 savedProj = Renderer::GetProjectionMatrix();

	// Light View/Proj を作る
	BuildSunShadowMatrices(m_LightView, m_LightProj);

	// 影用に Renderer の View/Proj を差し替える（b1,b2を影用にする）
	Renderer::SetViewMatrix(&m_LightView);
	Renderer::SetProjectionMatrix(&m_LightProj);

	// ShadowMapへ描く
	m_Shadow.Begin(ctx);

	// GS等を外す（影は深度だけ）
	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->HSSetShader(nullptr, nullptr, 0);
	ctx->DSSetShader(nullptr, nullptr, 0);

	// Opaqueだけを影に入れる（必要なら TransparentForward も影に入れる）
	for (const auto& p : m_Packets)
	{
		if (p.phase != RenderPhase::OpaqueGBuffer) continue;
		if (p.type != DrawType::Mesh) continue;

		const MeshDraw& md = std::get<MeshDraw>(p.payload);
		if (!md.vb || !md.ib) continue;

		UINT offset = 0;
		ctx->IASetPrimitiveTopology(md.topology);
		ctx->IASetVertexBuffers(0, 1, &md.vb, &md.stride, &offset);
		ctx->IASetIndexBuffer(md.ib, md.indexFormat, 0);

		// skinned判定
		bool isSkinned = false;
		for (const auto& di : md.items) { if (di.bones) { isSkinned = true; break; } }

		(isSkinned ? m_pShadowSkin : m_pShadowStatic)->SetGPU();
		// 深度のみなのでピクセルシェーダ無し
		ctx->PSSetShader(nullptr, nullptr, 0);

		Matrix4x4 w = md.world;
		Renderer::SetWorldMatrix(&w);

		for (const auto& di : md.items)
		{
			if (di.bones) di.bones->SetGPU(); // b5
			ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
		}
	}

	m_Shadow.End(ctx);

	// backbufferへ戻す
	Renderer::BindBackbuffer(true);

	// View/Proj をカメラに戻す
	Renderer::SetViewMatrix(&savedView);
	Renderer::SetProjectionMatrix(&savedProj);

	// LightViewProjT を保存（DeferredLightingで使う）
	Matrix4x4 lightVP = m_LightView * m_LightProj;
	m_LightViewProjT = lightVP.Transpose();
}


void RenderManager::BuildSunShadowMatrices(Matrix4x4& outView, Matrix4x4& outProj) const
{
	// 光が飛んでくる向き（Renderer::SetLight に入れてるやつ）
	LIGHT L = Renderer::GetLight();
	Vector3 lightDir(L.Direction.x, L.Direction.y, L.Direction.z);
	if (lightDir.LengthSquared() < 1e-6f) lightDir = Vector3(0, -1, 0);
	lightDir.Normalize();

	// カメラ位置（RendererのViewから逆算）
	Matrix4x4 camView = Renderer::GetViewMatrix();
	Matrix4x4 camInv = camView.Invert();
	Vector3 camPos = camInv.Translation();

	// 影を張る中心（とりあえずカメラ周辺）
	Vector3 center = camPos;

	// ライト位置：中心から逆方向へ離す
	const float lightDist = 8000.0f;
	Vector3 eye = center - lightDir * lightDist;

	// up（真上向きと平行に近いと壊れるので保険）
	Vector3 up = Vector3::Up;
	if (fabs(lightDir.Dot(up)) > 0.98f)
		up = Vector3(0, 0, 1);

	outView = DirectX::XMMatrixLookAtLH(eye, center, up);

	// Orthographic（範囲は調整）
	const float range = 6000.0f;
	const float nearZ = 1.0f;
	const float farZ = 20000.0f;

	outProj = DirectX::XMMatrixOrthographicOffCenterLH(
		-range, range,
		-range, range,
		nearZ, farZ
	);
}

void RenderManager::AssignSpotShadowSlices(LightSystem& ls, const Vector3& refPos,
	std::vector<SpotLightGPU>& outLights,
	std::array<int, SPOT_SHADOW_K>& outShadowSrcIndex,
	int& outShadowCount)
{
	outLights.clear();
	outLights.reserve(ls.GetSpotCount());

	// まず全部コピー（shadowSlice=-1のまま）
	for (int i = 0; i < ls.GetSpotCount(); ++i)
		outLights.push_back(ls.GetSpotGPU(i));

	// 距離でソート（元のインデックスも欲しい）
	struct Cand { float d2; int idx; };
	std::vector<Cand> cands;
	cands.reserve(outLights.size());

	for (int i = 0; i < (int)outLights.size(); ++i)
	{
		const auto& p = outLights[i].Position;
		Vector3 lp(p.x, p.y, p.z);
		float d2 = (lp - refPos).LengthSquared();
		cands.push_back({ d2, i });
	}
	std::sort(cands.begin(), cands.end(), [](auto& a, auto& b) { return a.d2 < b.d2; });

	// 先頭K本に slice 割り当て
	outShadowCount = std::min((int)cands.size(), SPOT_SHADOW_K);
	for (int s = 0; s < outShadowCount; ++s)
	{
		int li = cands[s].idx;
		outLights[li].Params2.z = (float)s;   // shadowSlice = 0..K-1
		outShadowSrcIndex[s] = li;            // 「outLights内でのインデックス」
	}
}


bool RenderManager::CreateSpotStructuredBuffer(ID3D11Device* dev, int capacity)
{
	m_SpotSB_Capacity = capacity;

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(SpotLightGPU) * capacity;
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(SpotLightGPU);

	if (FAILED(dev->CreateBuffer(&bd, nullptr, m_SpotSB.GetAddressOf())))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sd.Format = DXGI_FORMAT_UNKNOWN;
	sd.Buffer.FirstElement = 0;
	sd.Buffer.NumElements = capacity;

	if (FAILED(dev->CreateShaderResourceView(m_SpotSB.Get(), &sd, m_SpotSB_SRV.GetAddressOf())))
		return false;

	return true;
}

void RenderManager::UpdateSpotStructuredBuffer(ID3D11DeviceContext* ctx, const std::vector<SpotLightGPU>& lights)
{
	const int n = (int)lights.size();
	// capacity超えたら切る（ここはログ出しても良い）
	const int copyN = std::min(n, m_SpotSB_Capacity);

	D3D11_MAPPED_SUBRESOURCE ms{};
	ctx->Map(m_SpotSB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, lights.data(), sizeof(SpotLightGPU) * copyN);
	ctx->Unmap(m_SpotSB.Get(), 0);

	m_SpotCountThisFrame = copyN; // cbで渡す用
}

bool RenderManager::CreateStructuredUAVBuffer(ID3D11Device* dev, UINT numElements,
	ComPtr<ID3D11Buffer>& outBuf,
	ComPtr<ID3D11UnorderedAccessView>& outUAV,
	ComPtr<ID3D11ShaderResourceView>& outSRV)
{
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(UINT) * numElements;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = sizeof(UINT);

	if (FAILED(dev->CreateBuffer(&bd, nullptr, outBuf.GetAddressOf()))) return false;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uvd{};
	uvd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uvd.Format = DXGI_FORMAT_UNKNOWN;
	uvd.Buffer.FirstElement = 0;
	uvd.Buffer.NumElements = numElements;

	if (FAILED(dev->CreateUnorderedAccessView(outBuf.Get(), &uvd, outUAV.GetAddressOf()))) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	sd.Format = DXGI_FORMAT_UNKNOWN;
	sd.Buffer.FirstElement = 0;
	sd.Buffer.NumElements = numElements;

	if (FAILED(dev->CreateShaderResourceView(outBuf.Get(), &sd, outSRV.GetAddressOf()))) return false;

	return true;
}

bool RenderManager::CreateSpotAccum(ID3D11Device* dev)
{
	D3D11_TEXTURE2D_DESC td{};
	td.Width = static_cast<UINT>(Window::GetInstance().GetWidth());
	td.Height = static_cast<UINT>(Window::GetInstance().GetHeight());
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(dev->CreateTexture2D(&td, nullptr, m_SpotAccumTex.GetAddressOf()))) return false;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uvd{};
	uvd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uvd.Format = td.Format;
	uvd.Texture2D.MipSlice = 0;
	if (FAILED(dev->CreateUnorderedAccessView(m_SpotAccumTex.Get(), &uvd, m_SpotAccumUAV.GetAddressOf()))) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Format = td.Format;
	sd.Texture2D.MostDetailedMip = 0;
	sd.Texture2D.MipLevels = 1;
	if (FAILED(dev->CreateShaderResourceView(m_SpotAccumTex.Get(), &sd, m_SpotAccumSRV.GetAddressOf()))) return false;

	return true;
}

bool RenderManager::CreateBeamTex(ID3D11Device* dev)
{
	D3D11_TEXTURE2D_DESC td{};
	td.Width = BEAM_W;
	td.Height = BEAM_H;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(dev->CreateTexture2D(&td, nullptr, m_BeamTex.GetAddressOf()))) return false;

	D3D11_UNORDERED_ACCESS_VIEW_DESC uvd{};
	uvd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uvd.Format = td.Format;
	if (FAILED(dev->CreateUnorderedAccessView(m_BeamTex.Get(), &uvd, m_BeamUAV.GetAddressOf()))) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Format = td.Format;
	sd.Texture2D.MipLevels = 1;
	if (FAILED(dev->CreateShaderResourceView(m_BeamTex.Get(), &sd, m_BeamSRV.GetAddressOf()))) return false;

	return true;
}

void RenderManager::RunBeamCompute(const CBDeferred& cbDeferred, const CBTileInfo& ti)
{
	auto* ctx = Renderer::GetDeviceContext();

	// DepthSRV を読むので DSV を外す（重要）
	ctx->OMSetRenderTargets(0, nullptr, nullptr);

	// FogCompositePS が t6 に BeamTex を刺してる場合があるので外す
	{
		ID3D11ShaderResourceView* nullPS = nullptr;
		ctx->PSSetShaderResources(6, 1, &nullPS);
	}

	// BeamTexを0クリア
	float clearF[4] = { 0,0,0,0 };
	ctx->ClearUnorderedAccessViewFloat(m_BeamUAV.Get(), clearF);

	m_pCSBeam->SetGPU();

	// SRV: t3 Depth, t6 SpotLights, t7 TileCount, t8 TileIndex
	ID3D11ShaderResourceView* t3 = Renderer::GetDepthSRV();
	ID3D11ShaderResourceView* t6 = m_SpotSB_SRV.Get();
	ID3D11ShaderResourceView* t7 = m_TileCountSRV.Get();
	ID3D11ShaderResourceView* t8 = m_TileIndexSRV.Get();

	ctx->CSSetShaderResources(3, 1, &t3);
	ctx->CSSetShaderResources(6, 1, &t6);
	ctx->CSSetShaderResources(7, 1, &t7);
	ctx->CSSetShaderResources(8, 1, &t8);

	// UAV: u0 BeamOut
	ID3D11UnorderedAccessView* u0 = m_BeamUAV.Get();
	ctx->CSSetUnorderedAccessViews(0, 1, &u0, nullptr);

	// Sampler: s0（必須）
	ID3D11SamplerState* s0 = Renderer::GetSamplerLinearClamp();
	ctx->CSSetSamplers(0, 1, &s0);

	// CBDeferred(b9)
	ctx->UpdateSubresource(m_CBDeferred.Get(), 0, nullptr, &cbDeferred, 0, 0);
	ID3D11Buffer* b9 = m_CBDeferred.Get();
	ctx->CSSetConstantBuffers(9, 1, &b9);

	// CBTileInfo(b1)
	ctx->UpdateSubresource(m_CBTileInfo.Get(), 0, nullptr, &ti, 0, 0);
	ID3D11Buffer* b1 = m_CBTileInfo.Get();
	ctx->CSSetConstantBuffers(1, 1, &b1);

	CBBeam cb{};
	if (m_pWeatherSystem)
	{
		const auto& bt = m_pWeatherSystem->GetBeamTuning();
		cb.beamMaxDist = bt.maxDist;
		cb.stepLenWanted = bt.stepLen;
		cb.kBeam = bt.kBeam;
		cb.beamTint = bt.tint;
		cb.MaxSteps = (uint32_t)bt.maxSteps;
	}
	else
	{
		cb.beamMaxDist = 12000.0f;
		cb.stepLenWanted = 10.0f;
		cb.kBeam = 0.0020f;
		cb.beamTint = 1.0f;
		cb.MaxSteps = 512;
	}
	cb.BeamSize = Vector2((float)BEAM_W, (float)BEAM_H);

	ctx->UpdateSubresource(m_CBBeam.Get(), 0, nullptr, &cb, 0, 0);
	ID3D11Buffer* b0 = m_CBBeam.Get();
	ctx->CSSetConstantBuffers(0, 1, &b0);

	// Dispatch
	UINT gx = (BEAM_W + 7) / 8;
	UINT gy = (BEAM_H + 7) / 8;
	ctx->Dispatch(gx, gy, 1);

	// 後片付け
	ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
	ctx->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

	ID3D11ShaderResourceView* nullSRV[10] = {};
	ctx->CSSetShaderResources(0, 10, nullSRV);

	ctx->CSSetShader(nullptr, nullptr, 0);

	SkyFogPass::SetBeamSRV(m_BeamSRV.Get());
}


void RenderManager::RunSpotCompute(const CBDeferred& cbDeferred, const Matrix4x4& viewT, float proj11, float proj22)
{
	auto* ctx = Renderer::GetDeviceContext();
	// Compute中はOM(描画先)を使わないので、RTV/DSVを外す
	ctx->OMSetRenderTargets(0, nullptr, nullptr);

	// 1) TileCountを0クリア
	UINT clearU[4] = { 0,0,0,0 };
	ctx->ClearUnorderedAccessViewUint(m_TileCountUAV.Get(), clearU);

	// 2) SpotAccumを0クリア
	float clearF[4] = { 0,0,0,0 };
	ctx->ClearUnorderedAccessViewFloat(m_SpotAccumUAV.Get(), clearF);

	// =========================
	// Pass A: BuildTileCS
	// =========================
	m_pCSBuildTile->SetGPU();

	// SRV(t6) = SpotLights SB
	ID3D11ShaderResourceView* srvSpot = m_SpotSB_SRV.Get();
	ctx->CSSetShaderResources(6, 1, &srvSpot);

	// UAV(u0,u1) = TileCount, TileIndex
	ID3D11UnorderedAccessView* uavsA[2] = { m_TileCountUAV.Get(), m_TileIndexUAV.Get() };
	ctx->CSSetUnorderedAccessViews(0, 2, uavsA, nullptr);

	// CBTile(b0)
	CBTile cbt{};
	cbt.ViewT = viewT; // 既にTranspose済みを渡す想定
	const float w = static_cast<float>(Window::GetInstance().GetWidth());
	const float h = static_cast<float>(Window::GetInstance().GetHeight());

	cbt.Screen = Vector2(w, h);
	cbt.ProjScale = Vector2(proj11, proj22);
	cbt.SpotCount = (uint32_t)m_SpotCountThisFrame;
	cbt.MaxPerTile = MAX_LIGHTS_PER_TILE;

	ctx->UpdateSubresource(m_CBTile.Get(), 0, nullptr, &cbt, 0, 0);
	ID3D11Buffer* b0 = m_CBTile.Get();
	ctx->CSSetConstantBuffers(0, 1, &b0);

	ctx->Dispatch(TILE_COUNT, 1, 1);

	// UAVを外す（次のパスのため）
	ID3D11UnorderedAccessView* nullUAV2[2] = { nullptr, nullptr };
	ctx->CSSetUnorderedAccessViews(0, 2, nullUAV2, nullptr);

	// SRVも外す（安全）
	ID3D11ShaderResourceView* nullSRV1[1] = { nullptr };
	ctx->CSSetShaderResources(6, 1, nullSRV1);

	// =========================
	// Pass B: SpotLightingCS
	// =========================
	m_pCSSpotLighting->SetGPU();

	// SRV: t0,t1,t3
	ID3D11ShaderResourceView* t0 = m_GBuffer.GetSRV(0);
	ID3D11ShaderResourceView* t1 = m_GBuffer.GetSRV(1);
	ID3D11ShaderResourceView* t3 = Renderer::GetDepthSRV();
	ctx->CSSetShaderResources(0, 1, &t0);
	ctx->CSSetShaderResources(1, 1, &t1);
	ctx->CSSetShaderResources(3, 1, &t3);

	// SRV: t6 SpotLights, t7 TileCountSRV, t8 TileIndexSRV
	ID3D11ShaderResourceView* t6 = m_SpotSB_SRV.Get();
	ID3D11ShaderResourceView* t7 = m_TileCountSRV.Get();
	ID3D11ShaderResourceView* t8 = m_TileIndexSRV.Get();
	ctx->CSSetShaderResources(6, 1, &t6);
	ctx->CSSetShaderResources(7, 1, &t7);
	ctx->CSSetShaderResources(8, 1, &t8);

	// UAV(u0) = SpotAccum
	ID3D11UnorderedAccessView* uavB[1] = { m_SpotAccumUAV.Get() };
	ctx->CSSetUnorderedAccessViews(0, 1, uavB, nullptr);

	// Samp(s0) をCSにも渡す（Depth/GBufferサンプル用）
	ID3D11SamplerState* s0 = Renderer::GetSamplerLinearClamp();
	ctx->CSSetSamplers(0, 1, &s0);	// 例：Rendererが持ってるサンプラを取れるならそれを。無いならRenderManagerで作る。

	// Samp(s1) 影用サンプラ
	ID3D11SamplerState* s1 = m_ShadowCmpSampler.Get();
	ctx->CSSetSamplers(1, 1, &s1);

	// CBDeferred(b9) をCSにもセット
	ctx->UpdateSubresource(m_CBDeferred.Get(), 0, nullptr, &cbDeferred, 0, 0);
	ID3D11Buffer* b9 = m_CBDeferred.Get();
	ctx->CSSetConstantBuffers(9, 1, &b9);

	// CBTileInfo(b1)
	CBTileInfo ti{};
	ti.SpotCount = (uint32_t)m_SpotCountThisFrame;
	ti.MaxPerTile = MAX_LIGHTS_PER_TILE;
	ti.TileW = TILE_W;
	ti.TileH = TILE_H;

	// t9: SpotShadowTex（配列）
	ID3D11ShaderResourceView* t9 = m_SpotShadow.GetSRV();
	ctx->CSSetShaderResources(9, 1, &t9);

	// b2: CBSpotShadow
	ID3D11Buffer* b2 = m_CBSpotShadow.Get();
	ctx->CSSetConstantBuffers(2, 1, &b2);

	ctx->UpdateSubresource(m_CBTileInfo.Get(), 0, nullptr, &ti, 0, 0);
	ID3D11Buffer* b1 = m_CBTileInfo.Get();
	ctx->CSSetConstantBuffers(1, 1, &b1);

	ctx->Dispatch(TILE_W, TILE_H, 1);

	// 後片付け（UAV/SRV解除）
	ID3D11UnorderedAccessView* nullUAV1[1] = { nullptr };
	ctx->CSSetUnorderedAccessViews(0, 1, nullUAV1, nullptr);

	ID3D11ShaderResourceView* nulls[10] = {};
	ctx->CSSetShaderResources(0, 10, nulls);

	ctx->CSSetShader(nullptr, nullptr, 0);
}


void RenderManager::BuildSpotShadowMatrices(const SpotLightGPU& s, Matrix4x4& outView, Matrix4x4& outProj)
{
	Vector3 pos(s.Position.x, s.Position.y, s.Position.z);
	Vector3 dir(s.Direction.x, s.Direction.y, s.Direction.z);
	if (dir.LengthSquared() < 1e-6f) dir = Vector3(0, -1, 0);
	dir.Normalize();

	Vector3 up = Vector3::Up;
	if (fabs(dir.Dot(up)) > 0.98f) up = Vector3(0, 0, 1);

	outView = DirectX::XMMatrixLookAtLH(pos, pos + dir, up);

	float outerCos = std::clamp(s.Params1.z, -1.0f, 1.0f);
	float fov = 2.0f * acosf(outerCos);
	fov = std::clamp(fov, 0.1f, 3.10f);

	float nearZ = std::max(0.1f, s.Params2.y);       // near
	float farZ = std::max(nearZ + 1.0f, s.Params1.x); // range

	outProj = DirectX::XMMatrixPerspectiveFovLH(fov, 1.0f, nearZ, farZ);
}

void RenderManager::RenderSpotShadowPass(const std::vector<SpotLightGPU>& lights,
	const std::array<int, SPOT_SHADOW_K>& shadowIdx,
	int shadowCount)
{
	auto* ctx = Renderer::GetDeviceContext();

	// 退避（カメラ）
	Matrix4x4 savedView = Renderer::GetViewMatrix();
	Matrix4x4 savedProj = Renderer::GetProjectionMatrix();

	Renderer::SetDepthEnable(true);
	Renderer::SetBlendState(BS_NONE);

	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->HSSetShader(nullptr, nullptr, 0);
	ctx->DSSetShader(nullptr, nullptr, 0);

	// K本ぶん描く（0..shadowCount-1 が slice）
	for (int si = 0; si < shadowCount; ++si)
	{
		int li = shadowIdx[si];          // lights のインデックス
		const SpotLightGPU& s = lights[li];

		Matrix4x4 lv, lp;
		BuildSpotShadowMatrices(s, lv, lp);

		Renderer::SetViewMatrix(&lv);
		Renderer::SetProjectionMatrix(&lp);

		m_SpotShadow.BeginSlice(ctx, si);

		for (const auto& p : m_Packets)
		{
			if (p.phase != RenderPhase::OpaqueGBuffer) continue;
			if (p.type != DrawType::Mesh) continue;

			const MeshDraw& md = std::get<MeshDraw>(p.payload);
			if (!md.vb || !md.ib) continue;

			UINT offset = 0;
			ctx->IASetPrimitiveTopology(md.topology);
			ctx->IASetVertexBuffers(0, 1, &md.vb, &md.stride, &offset);
			ctx->IASetIndexBuffer(md.ib, md.indexFormat, 0);

			bool isSkinned = false;
			for (const auto& di : md.items) if (di.bones) { isSkinned = true; break; }
			(isSkinned ? m_pShadowSkin : m_pShadowStatic)->SetGPU();

			ctx->PSSetShader(nullptr, nullptr, 0); // 深度のみ

			Matrix4x4 w = md.world;
			Renderer::SetWorldMatrix(&w);

			for (const auto& di : md.items)
			{
				if (di.bones) di.bones->SetGPU();
				ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
			}
		}

		// VP^T を保存（HLSLが row-vector mul(world, VP_T) の前提）
		Matrix4x4 vp = lv * lp;
		m_SpotShadowCB.SpotLightViewProjT[si] = vp.Transpose();
	}

	// 未使用を埋める
	for (int i = shadowCount; i < SPOT_SHADOW_K; ++i)
		m_SpotShadowCB.SpotLightViewProjT[i] = Matrix4x4::Identity;

	// 戻す
	Renderer::SetViewMatrix(&savedView);
	Renderer::SetProjectionMatrix(&savedProj);
	Renderer::BindBackbuffer(true);

	// b2 を更新
	m_SpotShadowCB.SpotShadowTexel =
		Vector4(1.0f / m_SpotShadow.W(), 1.0f / m_SpotShadow.H(),
			(float)m_SpotShadow.W(), (float)m_SpotShadow.H());

	// 初期値（まずこれ）
	float bias = 0.0025f;
	float normalBias = 0.008f;
	float pcfRadius = 1.0f; // 3x3

	m_SpotShadowCB.SpotShadowParams =
		Vector4(bias, normalBias, pcfRadius, (float)shadowCount);

	ctx->UpdateSubresource(m_CBSpotShadow.Get(), 0, nullptr, &m_SpotShadowCB, 0, 0);
}