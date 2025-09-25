#pragma once
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "system/CVertexBuffer.h"
#include "system/CIndexBuffer.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Graphics/RenderInfo.h"

/// <summary>
/// メッシュ描画用のレンダラー
/// </summary>
template <typename T>
class MeshRenderer final : public IRenderer
{
public:
	MeshRenderer(GameObject* owner) : IRenderer(owner) {};
	~MeshRenderer() {
		m_VSName = nullptr;
		m_PSName = nullptr;
	};

	void SetVertexBuffer(const CVertexBuffer<T>& vb) { m_VertexBuffer = vb; }
	void SetIndexBuffer(const CIndexBuffer& ib) { m_IndexBuffer = ib; }

	void SetShaders(const char* vsName, const char* psName)
	{
		m_VSName = vsName;
		m_PSName = psName;
	}

	bool GetRenderInfo(RenderInfo& outInfo) override
	{
		// 所有者がいない/シェーダー名が設定されていない場合は描画しない
		if (!m_pOwner || !m_VsName || !m_PSName) { return false; }

		// ワールド行列を取得
		Matrix4x4* mtx = m_pOwner->GetWorldMatrix();

		// 描画情報をセット
		outInfo.vertexBuffer = m_VertexBuffer.;

	}

private:
	CVertexBuffer<T>	m_VertexBuffer;		// 頂点バッファ
	CIndexBuffer		m_IndexBuffer;		// インデックスバッファ

	const char* m_VSName = nullptr;
	const char* m_PSName = nullptr;
};
