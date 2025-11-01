#pragma once
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "system/CVertexBuffer.h"
#include "system/CIndexBuffer.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Graphics/RenderInfo.h"
#include "system/CMaterial.h"

/*
* @brief	メッシュ描画コンポーネント
* @detail	メッシュを描画するためのコンポーネント
* @remark	頂点バッファ、インデックスバッファ、シェーダー名を保持する
* @remark	メッシュデータをポインタで持ち、そのデータを元に頂点バッファ、インデックスバッファを生成、描画を行う
* @auther	赤根和樹
* @date		2025/10/16
* 
* todo: マテリアル、テクスチャ、シェーダーをRenderInfoに入れてRendereManagerから描画、で行けそう
*/
class MeshRenderer final : public IRenderer
{
public:
	MeshRenderer() : IRenderer() {};
	~MeshRenderer() = default;

	void Init()

	/*void SetVertexBuffer(const CVertexBuffer<VERTEX_3D>& vb) { m_VertexBuffer = vb; }
	void SetIndexBuffer(const CIndexBuffer& ib) { m_IndexBuffer = ib; }*/

	//bool GetRenderInfo(RenderInfo& outInfo) override
	//{
	//	// 所有者がいない/シェーダー名が設定されていない場合は描画しない
	//	if (!m_pOwner || !m_VSName || !m_PSName) { return false; }

	//	// ワールド行列を取得
	//	Matrix4x4* mtx = &m_pOwner->GetWorldMatrix();

	//	// 描画必要情報をセット
	//	outInfo.vertexBuffer = m_VertexBuffer.GetBuffer();
	//	outInfo.indexBuffer = m_IndexBuffer.GetBuffer();
	//	outInfo.stride = m_VertexBuffer.GetStride();
	//	outInfo.indexCount = m_IndexBuffer.GetIndexCount();
	//	outInfo.indexFormat = m_IndexBuffer.GetIndexFormat();
	//	outInfo.world = mtx;
	//	outInfo.VSName = m_VSName;
	//	outInfo.psName = m_PSName;

	//	return true;
	//}

private:
	CVertexBuffer<VERTEX_3D>	m_VertexBuffer;	// 頂点バッファ
	CIndexBuffer				m_IndexBuffer;	// インデックスバッファ
	std::unique_ptr<CMaterial> m_pMaterial;		// マテリアル
};
