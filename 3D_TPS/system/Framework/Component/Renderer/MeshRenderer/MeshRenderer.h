#pragma once
#include "system/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "system/MyVertexBuffer.h"
#include "system/MyIndexBuffer.h"
//#include "system/CVertexBuffer.h"
//#include "system/CIndexBuffer.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Graphics/RenderInfo.h"
#include "Material/MyMaterial.h"
//#include "system/CMaterial.h"


struct MeshRenderData {
	std::vector<VERTEX_3D> vertices;
	std::vector<uint32_t> indices;
	std::vector<MATERIAL> m_materials;					// マテリアル情報
	std::vector<std::string> m_diffusetexturenames;		// ディフューズテクスチャ名
	std::vector<SUBSET> m_subsets;						// サブセット情報

	std::vector<std::unique_ptr<CTexture>>	m_diffusetextures;	// ディフューズテクスチャ群
};


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

	void Init(void) override;
	void Update(const uint64_t deltatime) override;
	void Uninit(void) override;

	bool GetRenderInfo(RenderInfo& outInfo) override;
	void EnumerateRenderInfos(std::vector<RenderInfo>& infos) override;

private:
	MyVertexBuffer<VERTEX_3D>	m_VertexBuffer;	// 頂点バッファ
	MyIndexBuffer				m_IndexBuffer;	// インデックスバッファ
	
	std::vector<SUBSET> m_Subsets;
	std::vector<ID3D11ShaderResourceView*> m_Textures;
	std::vector<std::unique_ptr<MyMaterial>> m_Materials;

	ComPtr<ID3D11Buffer> m_pWorldBuffer;		// ワールド行列バッファ

	MeshRenderData m_MeshData;
};
