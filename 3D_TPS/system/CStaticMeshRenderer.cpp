#include "CStaticMeshRenderer.h"
#include <crtdbg.h>

void CStaticMeshRenderer::Init(CStaticMesh& mesh)
{
    _CrtCheckMemory(); // ① ここで壊れてたら「この前」がおかしい

    m_Materiales.clear();
    m_Subsets.clear();
    m_DiffuseTextures.clear();

    CMeshRenderer::Init(mesh);
    _CrtCheckMemory(); // ② ここで壊れたら CMeshRenderer::Init が犯人

    m_Subsets = mesh.GetSubsets();
    m_DiffuseTextures = mesh.GetDiffuseTextures();

    auto materials = mesh.GetMaterials();
    _CrtCheckMemory(); // ③ ここで壊れたら GetMaterials が犯人

    if (materials.size() > 4096) __debugbreak();

    m_Materiales.reserve(materials.size());

    for (size_t i = 0; i < materials.size(); ++i)
    {
        auto m = std::make_unique<CMaterial>();

        _CrtCheckMemory(); // ④ Create 前
        m->Create(materials[i]);
        _CrtCheckMemory(); // ⑤ Create 後に壊れてたら Create が犯人

        m_Materiales.push_back(std::move(m));
        _CrtCheckMemory(); // ⑥ push_back 後
    }
}

//void CStaticMeshRenderer::Init(CStaticMesh& mesh)
//{
//	// 再Init対策
//	m_Materiales.clear();
//	m_Subsets.clear();
//	m_DiffuseTextures.clear();
//
//	// 頂点バッファとインデックスバッファを生成
//	CMeshRenderer::Init(mesh);
//
//	// サブセット情報取得
//	m_Subsets = mesh.GetSubsets();
//
//	// diffuseテクスチャ情報取得
//	m_DiffuseTextures = mesh.GetDiffuseTextures();
//
//	// マテリアル情報取得	
//	std::vector<MATERIAL> materials;
//	materials = mesh.GetMaterials();
//
//	// マテリアル数分ループしてマテリアルデータを生成
//	for (int i = 0; i < materials.size(); i++)
//	{
//		// マテリアルオブジェクト生成
//		std::unique_ptr<CMaterial> m = std::make_unique<CMaterial>();
//
//		// マテリアル情報をセット
//		m->Create(materials[i]);
//
//		// マテリアルオブジェクトを配列に追加
//		m_Materiales.push_back(std::move(m));
//	}
//}

void CStaticMeshRenderer::Draw()
{
	// インデックスバッファ・頂点バッファをセット
	BeforeDraw();

	// マテリアル数分ループ 
	for (int i = 0; i < m_Subsets.size(); i++)
	{
		// マテリアルをセット(サブセット情報の中にあるマテリアルインデックを使用する)
		m_Materiales[m_Subsets[i].MaterialIdx]->SetGPU();

		if (m_Materiales[m_Subsets[i].MaterialIdx]->isDiffuseTextureEnable())
		{
			m_DiffuseTextures[m_Subsets[i].MaterialIdx]->SetGPU();
		}

		//ShaderManager::GetInstance().GetShader("vertexLightingOneSkinVS")->Bind();
		//ShaderManager::GetInstance().GetShader("vertexLightingPS")->Bind();

		// サブセットの描画
		DrawSubset(
			m_Subsets[i].IndexNum,							// 描画するインデックス数
			m_Subsets[i].IndexBase,							// 最初のインデックスバッファの位置	
			m_Subsets[i].VertexBase);						// 頂点バッファの最初から使用
	}
}
