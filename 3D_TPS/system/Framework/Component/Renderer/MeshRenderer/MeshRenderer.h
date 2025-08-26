#pragma once
#include "Src/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "Mesh.h"

// 流れ
// GM：メッシュ生成→アニメーションオブジェクト生成
// →アニメーションメッシュをセット→アニメーションデータを読み込み→アニメーションデータセット

/// <summary>
/// メッシュ描画用のレンダラー
/// </summary>
class MeshRenderer final : public IRenderer
{
public:
	MeshRenderer();
	~MeshRenderer();

private:
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;		// 頂点バッファ
	IndexBuffer				m_IndexBuffer;		// インデックスバッファ
};

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
}