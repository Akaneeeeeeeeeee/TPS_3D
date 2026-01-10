#include "RenderManager.h"
#include "system/Framework/Graphics/GraphicsDevice.h"
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "system/Framework/Window/Window.h"
#include "system/CShader.h"
#include "system/CTexture.h"

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

	// DI
	//this->m_pGraphicsDevice = graphicsDevice;

	auto* dev = Renderer::GetDevice();
	m_GBuffer.Create(dev, Window::GetInstance().GetWidth(), Window::GetInstance().GetHeight());

	// gbufferVS/PS は main エントリでOK
	//m_pGBufferShader.Create("shader/gbufferVS.hlsl", "shader/gbufferPS.hlsl");
	return true;
}

void RenderManager::Uninit(void)
{
	// 描画コンポーネントリストと描画情報コンテナを単純にクリア（各コンポーネントのUninitは呼ばない）
	m_RenderComponents.clear();
	m_RenderInfos.clear();

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
void RenderManager::CollectRenderInfo(void)
{
	m_RenderInfos.clear(); // 前のフレームの描画情報をクリア
	for (auto& component : m_RenderComponents)
	{
		// コンポーネントが存在し、有効な場合のみ描画情報を取得
		if (component && component->GetIsValid())
		{
			RenderInfo info;
			// 取得できた場合は描画情報リストに追加
			if (component->GetRenderInfo(info))
			{
				m_RenderInfos.push_back(info); // 描画情報をリストに追加
			}
		}
	}
}

// 描画コンポーネント1つ分の描画
//void RenderManager::Render(const RenderInfo& info)
//{
//	auto context = m_pGraphicsDevice->GetContext();
//	// 頂点バッファの設定
//	UINT offset = 0;
//	context->IASetVertexBuffers(0, 1, &info.vertexBuffer, &info.stride, &offset);
//	// インデックスバッファの設定
//	context->IASetIndexBuffer(info.indexBuffer, info.indexFormat, 0);
//	// ワールド変換行列をシェーダーに渡す（将来: 定数バッファにまとめて渡す）
//	// 将来: シェーダーマネージャーからシェーダーを取得してセットする
//	if (info.vs) {
//		info.vs->Bind();
//	}
//	if (info.ps) {
//		info.ps->Bind();
//	}
//	// 描画コール
//	context->DrawIndexed(info.indexCount, 0, 0);
//	// シェーダーのアンバインド（将来: シェーダーマネージャーにアンバインド処理を追加する）
//	if (info.vs) {
//		info.vs->Unbind();
//	}
//	if (info.ps) {
//		info.ps->Unbind();
//	}
//}

void RenderManager::Render(const RenderInfo& info)
{
	if (!info.vertexBuffer || !info.indexBuffer || !info.items || !info.shader) { return; }

	//auto* ctx = m_pGraphicsDevice->GetContext();
	auto* ctx = Renderer::GetDeviceContext();

	UINT offset = 0;
	ctx->IASetPrimitiveTopology(info.topology);
	ctx->IASetVertexBuffers(0, 1, &info.vertexBuffer, &info.stride, &offset);
	ctx->IASetIndexBuffer(info.indexBuffer, info.indexFormat, 0);

	// シェーダセット（VS/PS/GS/レイアウト）
	info.shader->SetGPU();

	// world
	Matrix4x4 w = info.world;
	Renderer::SetWorldMatrix(&w);

	// phase
	if (info.phase == RenderPhase::OpaqueGBuffer)
	{
		Renderer::SetDepthEnable(true);
		Renderer::SetBlendState(BS_NONE);
	}
	else
	{
		Renderer::SetDepthEnable(true);          // 半透明で深度テストしたいならtrue
		Renderer::SetBlendState(BS_ALPHABLEND);  // 透過
	}

	for (const auto& di : *info.items)
	{
		if (di.bones)   
		{
			di.bones->SetGPU();   // b5
			di.bones->Update(); // 忘れずに
		}
		if (di.material) di.material->SetGPU(); // b3
		if (di.diffuse)  di.diffuse->SetGPU();  // SRV

		ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
	}

	// 透過のあとに戻すなら（安全側）
	if (info.phase == RenderPhase::TransparentForward)
	{
		Renderer::SetBlendState(BS_NONE);
	}
}



/// <summary>
/// @brief 登録されている全ての描画コンポーネントを描画する
/// 各コンポーネントのRender()メソッドを呼び出すことで、実際の描画処理を行う
/// 
/// 所有オブジェクトからタグを見て描画順変えれそう
/// 描画順のソートもここで行うことができるが、現在は単純に登録された順に描画している
/// </summary>
void RenderManager::RenderAll(void)
{
	// 描画情報リストをループして各コンポーネントの描画を実行
	for (auto& info : m_RenderInfos)
	{
		this->Render(info);
	}
}

void RenderManager::RenderDeferred()
{
	CollectRenderInfo();

	RenderGBufferPass();     // 2) 不透明を書き溜め
	RenderLightingPass();    // 3) ライトを当てる

	// 4) 透明（必要なら m_RenderInfos の TransparentForward をここで従来描画）
}

void RenderManager::RenderGBufferPass()
{
	auto* ctx = Renderer::GetDeviceContext();

	// Depthは Renderer が持ってる DSV を使う（今の設計のまま）
	// もし DSV getter が無いなら Renderer に GetDSV() を1本足すのが安全
	// 今回は OMGetRenderTargets で拾うより、Renderer側に GetDSV() を足すのが推奨。
	ID3D11RenderTargetView* dummyRTV = nullptr;
	ID3D11DepthStencilView* dsv = nullptr;
	ctx->OMGetRenderTargets(1, &dummyRTV, &dsv);

	m_GBuffer.Begin(ctx, dsv);

	// GBufferではライト不要、半透明も不要
	Renderer::SetDepthEnable(true);
	Renderer::SetBlendState(BS_NONE);

	// GBuffer用シェーダをセット
	m_pGBufferShader->SetGPU();

	// infoを回す
	for (const auto& ri : m_RenderInfos)
	{
		if (ri.phase != RenderPhase::OpaqueGBuffer) continue;
		if (!ri.vertexBuffer || !ri.indexBuffer || !ri.items) continue;

		UINT offset = 0;
		ctx->IASetPrimitiveTopology(ri.topology);
		ctx->IASetVertexBuffers(0, 1, &ri.vertexBuffer, &ri.stride, &offset);
		ctx->IASetIndexBuffer(ri.indexBuffer, ri.indexFormat, 0);

		// 既存の b0/b1/b2 を使う（Renderer::SetViewMatrix/SetProjectionMatrix はどこかで済んでる前提）
		Matrix4x4 w = ri.world;
		Renderer::SetWorldMatrix(&w);

		for (const auto& di : *ri.items)
		{
			if (di.bones) di.bones->SetGPU();
			if (di.material) di.material->SetGPU();
			if (di.diffuse)  di.diffuse->SetGPU();

			ctx->DrawIndexed(di.indexNum, di.indexBase, di.vertexBase);
		}
	}

	// 参照カウント解放（OMGetRenderTargetsしたので）
	if (dummyRTV) dummyRTV->Release();
	if (dsv) dsv->Release();
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
	Renderer::SetBlendState(BS_NONE);

	ctx->Draw(3, 0);

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

