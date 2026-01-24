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
}

RenderManager::RenderManager()
	: m_pGraphicsDevice(nullptr)
{
}

RenderManager::~RenderManager()
{
	this->Uninit();
}

// 初期化処理
bool RenderManager::Init(GraphicsDevice* graphicsDevice)
{
	if (graphicsDevice == nullptr) { return false; }

	// GBufferの初期化
	auto* dev = Renderer::GetDevice();
	m_GBuffer.Create(dev, Window::GetInstance().GetWidth(), Window::GetInstance().GetHeight());

	if(!m_Shadow.Create(dev, 2048, 2048))
	{
		return false;
	}

	// Deferred用シェーダーの初期化
	InitDeferredShaders();
	return true;
}

void RenderManager::Uninit(void)
{
	// 描画コンポーネントリストと描画情報コンテナを単純にクリア（各コンポーネントのUninitは呼ばない）
	m_RenderComponents.clear();
	m_Packets.clear();

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

// 描画コンポーネント1つ分の描画
//void RenderManager::Render(const RenderInfo& info)
//{
//	if (!info.vertexBuffer || !info.indexBuffer || !info.items || !info.shader) { return; }
//
//	//auto* ctx = m_pGraphicsDevice->GetContext();
//	auto* ctx = Renderer::GetDeviceContext();
//
//	UINT offset = 0;
//	ctx->IASetPrimitiveTopology(info.topology);
//	ctx->IASetVertexBuffers(0, 1, &info.vertexBuffer, &info.stride, &offset);
//	ctx->IASetIndexBuffer(info.indexBuffer, info.indexFormat, 0);
//
//	// シェーダセット（VS/PS/GS/レイアウト）
//	info.shader->SetGPU();
//
//	// world
//	Matrix4x4 w = info.world;
//	Renderer::SetWorldMatrix(&w);
//
//	// phaseのみ
//	if (info.phase == RenderPhase::OpaqueGBuffer)
//	{
//		Renderer::SetDepthEnable(true);
//		Renderer::SetBlendState(BS_NONE);
//	}
//
//	for (const auto& di : *info.items)
//	{
//		if (di.bones)	 di.bones->SetGPU();	// b5
//		if (di.material) di.material->SetGPU(); // b3
//		if (di.diffuse)  di.diffuse->SetGPU();  // SRV
//
//		ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
//	}
//}
//
//
//
///// <summary>
///// @brief 登録されている全ての描画コンポーネントを描画する
///// 各コンポーネントのRender()メソッドを呼び出すことで、実際の描画処理を行う
///// 
///// 所有オブジェクトからタグを見て描画順変えれそう
///// 描画順のソートもここで行うことができるが、現在は単純に登録された順に描画している
///// </summary>
//void RenderManager::RenderAll(void)
//{
//	// 描画情報リストをループして各コンポーネントの描画を実行
//	for (auto& info : m_RenderInfos)
//	{
//		this->Render(info);
//	}
//}

void RenderManager::RenderDeferred()
{
	CollectRenderPackets();

	// 1) 太陽影
	RenderShadowPass();

	// 2) 不透明を書き溜め
	RenderGBufferPass();

	// 3) ライトを当てる（ここで ShadowMap を参照）
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
			2.0f
		);

		ctx->UpdateSubresource(m_CBShadow.Get(), 0, nullptr, &s, 0, 0);
		ID3D11Buffer* cb10 = m_CBShadow.Get();
		ctx->VSSetConstantBuffers(10, 1, &cb10);
		ctx->PSSetConstantBuffers(10, 1, &cb10);
	}

	// SRV bind（t0..t3 + t4）
	ID3D11ShaderResourceView* srvs[5] = {
		m_GBuffer.GetSRV(0),
		m_GBuffer.GetSRV(1),
		m_GBuffer.GetSRV(2),
		Renderer::GetDepthSRV(),
		m_Shadow.GetSRV()
	};
	ctx->PSSetShaderResources(0, 5, srvs);

	// 影用サンプラを s1 にセット
	ID3D11SamplerState* shadowSamp = m_ShadowCmpSampler.Get();
	ctx->PSSetSamplers(1, 1, &shadowSamp);

	Renderer::SetDepthEnable(false);
	Renderer::SetBlendState(BS_ALPHABLEND);

	ctx->Draw(3, 0);

	Renderer::SetBlendState(BS_NONE);
	Renderer::SetDepthEnable(true);

	// SRV解除
	ID3D11ShaderResourceView* nulls[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	ctx->PSSetShaderResources(0, 5, nulls);
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

	// ラスタ：カリングは好み。まずは Front を消す/Back を消すで試す

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
