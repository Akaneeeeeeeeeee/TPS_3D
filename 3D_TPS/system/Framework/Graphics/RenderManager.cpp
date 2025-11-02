#include "RenderManager.h"
#include "system/Framework/Graphics/GraphicsDevice.h"
#include "system/Framework/ShaderManager/ShaderManager.h"
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"

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
	this->m_pGraphicsDevice = graphicsDevice;
	return true;	// 初期化成功
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
	m_pGraphicsDevice->StartRender();
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
void RenderManager::Render(const RenderInfo& info)
{
	auto context = m_pGraphicsDevice->GetContext();
	// 頂点バッファの設定
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &info.vertexBuffer, &info.stride, &offset);
	// インデックスバッファの設定
	context->IASetIndexBuffer(info.indexBuffer, info.indexFormat, 0);
	// ワールド変換行列をシェーダーに渡す（将来: 定数バッファにまとめて渡す）
	// 将来: シェーダーマネージャーからシェーダーを取得してセットする
	if (info.vs) {
		info.vs->Bind();
	}
	if (info.ps) {
		info.ps->Bind();
	}
	// 描画コール
	context->DrawIndexed(info.indexCount, 0, 0);
	// シェーダーのアンバインド（将来: シェーダーマネージャーにアンバインド処理を追加する）
	if (info.vs) {
		info.vs->Unbind();
	}
	if (info.ps) {
		info.ps->Unbind();
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

// 描画終了処理
void RenderManager::EndRender(void)
{
	m_pGraphicsDevice->FinishRender();
}

