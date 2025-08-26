#include "RenderManager.h"
#include "Src/Framework/Graphics/GraphicsDevice.h"
#include "Src/Framework/Component/Renderer/IRenderer/IRenderer.h"

// 初期化処理
bool RenderManager::Init(GraphicsDevice* graphicsDevice, ShaderManager* shaderMgr)
{
	if (graphicsDevice == nullptr) return false;
	this->m_pGraphicsDevice = graphicsDevice;
	this->m_pShaderManager = shaderMgr;
	return true;	// 初期化成功
}

void RenderManager::Uninit(void)
{
	// 描画コンポーネントリストを単純にクリア（各コンポーネントのUninitは呼ばない）
	m_RenderComponents.clear();
	m_pGraphicsDevice = nullptr;	// GraphicsDeviceへのポインタをクリア
}

// 描画コンポーネントの登録
void RenderManager::RegisterRenderComponent(IRenderer* component)
{
	if (component == nullptr) return;
	this->m_RenderComponents.push_back(component);
}

// 描画コンポーネントの解除
void RenderManager::UnregisterRenderComponent(IRenderer* component)
{
	this->m_RenderComponents.erase(
		std::remove(this->m_RenderComponents.begin(), this->m_RenderComponents.end(), component),
		this->m_RenderComponents.end());
}

// 描画開始処理
void RenderManager::StartRender(void)
{
	m_pGraphicsDevice->StartRender();
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
	for(auto& component : m_RenderComponents) {
		if (component) {
			component->Render(); // 各コンポーネントのRenderメソッドを呼び出す
		}
	}
}

// 描画終了処理
void RenderManager::EndRender(void)
{
	m_pGraphicsDevice->FinishRender();
}
