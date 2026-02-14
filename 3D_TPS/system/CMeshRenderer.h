#pragma once
#include	"CVertexBuffer.h"
#include	"CIndexBuffer.h"
#include	"CMesh.h"

class CMeshRenderer {
protected:
	CVertexBuffer<VERTEX_SKINNED_GPU>	m_VertexBuffer;		// 頂点バッファ
	//CVertexBuffer<VERTEX_3D>	m_VertexBuffer;		// 頂点バッファ
	CIndexBuffer				m_IndexBuffer;		// インデックスバッファ
	int							m_IndexNum = 0;		// インデックス数
public:
	virtual ~CMeshRenderer() = default;

	virtual void Init(CMesh& mesh) 
	{
		m_VertexBuffer.Create(mesh.GetVertices());
		m_IndexBuffer.Create(mesh.GetIndices());
		m_IndexNum = static_cast<int>(mesh.GetIndices().size());
	}

	// 描画前処理
	virtual void BeforeDraw(D3D_PRIMITIVE_TOPOLOGY primtype) const
	{
		ID3D11DeviceContext* devicecontext;

		devicecontext = Renderer::GetDeviceContext();

		// トポロジーをセット（旧プリミティブタイプ）
		devicecontext->IASetPrimitiveTopology(primtype);

		m_VertexBuffer.SetGPU();			// 頂点バッファをセット
		m_IndexBuffer.SetGPU();				// インデックスバッファをセット
	}

	// 描画前処理
	virtual void BeforeDraw() const
	{
		ID3D11DeviceContext* devicecontext;

		devicecontext = Renderer::GetDeviceContext();

		// トポロジーをセット（旧プリミティブタイプ）
		devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_VertexBuffer.SetGPU();			// 頂点バッファをセット
		m_IndexBuffer.SetGPU();				// インデックスバッファをセット
	}

	// サブセット描画
	virtual void DrawSubset(unsigned int indexnum,unsigned int baseindex,unsigned int basevertexindex) const
	{
		Renderer::GetDeviceContext()->DrawIndexed(
			indexnum,								// 描画するインデックス数（面数×３）
			baseindex,								// 最初のインデックスバッファの位置
			basevertexindex);						// 頂点バッファの最初から使う
	}

	// 描画
	virtual void Draw() const
	{
		BeforeDraw();								// 描画前処理

		Renderer::GetDeviceContext()->DrawIndexed(
			m_IndexNum,								// 描画するインデックス数（面数×３）
			0,										// 最初のインデックスバッファの位置
			0);										// 頂点バッファの最初から使う
	}

	// 描画
	virtual void Draw(D3D_PRIMITIVE_TOPOLOGY primtype) const
	{
		BeforeDraw(primtype);								// 描画前処理

		Renderer::GetDeviceContext()->DrawIndexed(
			m_IndexNum,								// 描画するインデックス数（面数×３）
			0,										// 最初のインデックスバッファの位置
			0);										// 頂点バッファの最初から使う
		//Renderer::GetDeviceContext()->DrawIndexed(
		//	m_IndexNum * 2,								// 描画するインデックス数（面数×３）
		//	0,										// 最初のインデックスバッファの位置
		//	0);										// 頂点バッファの最初から使う
	}

	// インスタンシング描画
	virtual void DrawInstanced(UINT instanceCount) const
	{
		BeforeDraw(); // トポロジーと VB/IB セット

		Renderer::GetDeviceContext()->DrawIndexedInstanced(
			m_IndexNum,      // 1 インスタンスあたりのインデックス数
			instanceCount,   // インスタンス数
			0,               // StartIndexLocation
			0,               // BaseVertexLocation
			0                // StartInstanceLocation
		);
	}

	// インスタンシング描画（トポロジー指定版）
	virtual void DrawInstanced(D3D_PRIMITIVE_TOPOLOGY primtype, UINT instanceCount) const
	{
		// まず線用のトポロジーにする
		BeforeDraw(primtype);

		if (m_IndexNum <= 0 || instanceCount == 0) return;

		Renderer::GetDeviceContext()->DrawIndexedInstanced(
			m_IndexNum,
			instanceCount,
			0,
			0,
			0
		);
	}


	// 頂点バッファを更新
	void Modify(const std::vector<VERTEX_SKINNED_GPU>& vertices)
	{
		m_VertexBuffer.Modify(vertices);
	}
	/*void Modify(const std::vector<VERTEX_3D>& vertices)
	{
		m_VertexBuffer.Modify(vertices);
	}*/
	// 頂点バッファ・インデックスバッファを更新
	void Modify(const std::vector<VERTEX_SKINNED_GPU>& vertices, const std::vector<uint32_t>& indices)
	{
		m_VertexBuffer.Modify(vertices);
		m_IndexBuffer.Modify(indices);
	}
	/*void Modify(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices)
	{
		m_VertexBuffer.Modify(vertices);
		m_IndexBuffer.Modify(indices);
	}*/

	ID3D11Buffer* GetVB() const { return m_VertexBuffer.GetBuffer(); }
	ID3D11Buffer* GetIB() const { return m_IndexBuffer.GetBuffer(); }

	DXGI_FORMAT   GetIndexFormat() const { return m_IndexBuffer.GetFormat(); }
};
