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

static ComPtr<ID3DBlob> Compile(const wchar_t* path, const char* entry, const char* target)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> blob, err;
	HRESULT hr = D3DCompileFromFile(
		path,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry,
		target,
		flags,
		0,
		blob.GetAddressOf(),
		err.GetAddressOf()
	);

	if (FAILED(hr))
	{
		std::string msg = "D3DCompileFromFile failed.\n";
		msg += "file: ";
		// wchar->utf8 変換は面倒なので、とりあえず entry/target を出す
		msg += " entry: ";  msg += entry;
		msg += " target: "; msg += target;
		msg += "\n";

		if (err)
		{
			msg += "---- HLSL compiler log ----\n";
			msg += (const char*)err->GetBufferPointer();
			msg += "\n---------------------------\n";
		}
		throw std::runtime_error(msg);
	}
	return blob;
}

void RenderManager::InitDeferredShaders(void)
{
	auto* dev = Renderer::GetDevice();

	auto vs = Compile(L"shader/DeferredLighting.hlsl", "VS_Fullscreen", "vs_5_0");
	dev->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, m_FullVS.GetAddressOf());

	auto ps = Compile(L"shader/DeferredLighting.hlsl", "PS_Lighting", "ps_5_0");
	dev->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, m_LightPS.GetAddressOf());

	// GBuffer用シェーダー取得
	auto& am = AssetManager::GetInstance();
	m_pGBufferStatic = am.GetShader<CShader>("gbuffer_static");
	m_pGBufferSkin = am.GetShader<CShader>("gbuffer_skin");

	// シェーダー取得確認
	if (!m_pGBufferStatic || !m_pGBufferSkin)
	{
		throw std::runtime_error("RenderManager::InitDeferredShaders failed to get GBuffer shaders.");
	}

	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(CBDeferred);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dev->CreateBuffer(&bd, nullptr, m_CBDeferred.GetAddressOf());
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

	// 2) 不透明を書き溜め
	RenderGBufferPass();
	// 3) ライトを当てる
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

	// backbuffer に出す（DSVは不要）
	ID3D11RenderTargetView* bb = Renderer::GetRTV();
	ctx->OMSetRenderTargets(1, &bb, nullptr);

	D3D11_VIEWPORT vp{};
	vp.Width = (float)Window::GetInstance().GetWidth();
	vp.Height = (float)Window::GetInstance().GetHeight();
	vp.MinDepth = 0; vp.MaxDepth = 1;
	ctx->RSSetViewports(1, &vp);

	// フルスクリーンなので IA は空
	ctx->IASetInputLayout(nullptr);
	ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 残ってるGS等を外す（前に詰まった所）
	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->HSSetShader(nullptr, nullptr, 0);
	ctx->DSSetShader(nullptr, nullptr, 0);

	ctx->VSSetShader(m_FullVS.Get(), nullptr, 0);
	ctx->PSSetShader(m_LightPS.Get(), nullptr, 0);

	// CBDeferred 更新（invView/invProj/camPos）
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
	ID3D11Buffer* cbuf = m_CBDeferred.Get();
	ctx->VSSetConstantBuffers(9, 1, &cbuf);
	ctx->PSSetConstantBuffers(9, 1, &cbuf);

	// GBuffer SRV + DepthSRV を bind
	ID3D11ShaderResourceView* srvs[4] = {
		m_GBuffer.GetSRV(0),
		m_GBuffer.GetSRV(1),
		m_GBuffer.GetSRV(2),
		Renderer::GetDepthSRV()
	};
	ctx->PSSetShaderResources(0, 4, srvs);

	Renderer::SetDepthEnable(false);
	Renderer::SetBlendState(BS_ALPHABLEND);

	ctx->Draw(3, 0);
	Renderer::SetBlendState(BS_NONE);

	// SRV解除（次のパス衝突防止）
	ID3D11ShaderResourceView* nulls[4] = { nullptr, nullptr, nullptr, nullptr };
	ctx->PSSetShaderResources(0, 4, nulls);

	Renderer::SetDepthEnable(true);
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
	// ※CShader::SetGPU がGSもセットするなら、前のパスの残骸があっても上書きされる
	md.shader->SetGPU();

	// World
	Matrix4x4 w = md.world;
	Renderer::SetWorldMatrix(&w);

	// サブセット描画
	// items は std::span<const DrawItem> 前提（あなたの最新版）
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