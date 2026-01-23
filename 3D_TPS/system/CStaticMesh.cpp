#include    "commontypes.h"
#include	"CStaticMesh.h"
#include	"AssimpPerse.h"

void CStaticMesh::Load(std::string filename, std::string texturedirectory)
{
	std::vector<GM31::GE::myAssimp::SUBSET> subsets{};					// サブセット情報
	std::vector<std::vector<GM31::GE::myAssimp::VERTEX>> vertices{};	// 頂点データ（メッシュ単位）
	std::vector<std::vector<unsigned int>> indices{};					// インデックスデータ（メッシュ単位）
	std::vector<GM31::GE::myAssimp::MATERIAL> materials{};				// マテリアル
	std::vector<std::unique_ptr<CTexture>> embededtextures{};			// 内蔵テクスチャ群

	// assimpを使用してモデルデータを取得
	GM31::GE::myAssimp::GetModelData(filename, texturedirectory);

	subsets = GM31::GE::myAssimp::GetSubsets();							// サブセット情報取得
	vertices = GM31::GE::myAssimp::GetVertices();						// 頂点データ（メッシュ単位）
	indices = GM31::GE::myAssimp::GetIndices();							// インデックスデータ（メッシュ単位）
	materials = GM31::GE::myAssimp::GetMaterials();						// マテリアル情報取得

	m_diffusetextures = GM31::GE::myAssimp::GetDiffuseTextures();		// ｄｉｆｆｕｓｅテクスチャ情報取得	

	// 頂点データ作成
	int meshidx = 0;

	for (const auto& mv : vertices)
	{
		for (const auto& v : mv)
		{
			VERTEX_SKINNED_GPU out{};
			out.Position = { v.pos.x, v.pos.y, v.pos.z };
			out.Normal = { v.normal.x, v.normal.y, v.normal.z };
			out.TexCoord = { v.texcoord.x, v.texcoord.y };
			out.Diffuse = { v.color.r, v.color.g, v.color.b, v.color.a };

			// 4枠固定でゼロ初期化
			for (int i = 0; i < 4; ++i)
			{
				out.BoneIndex[i] = 0;
				out.BoneWeight[i] = 0.0f;
			}

			const int cnt = std::min(v.bonecnt, 4);
			for (int i = 0; i < cnt; ++i)
			{
				out.BoneIndex[i] = v.BoneIndex[i];
				out.BoneWeight[i] = v.BoneWeight[i];
			}

			m_vertices.emplace_back(out);
		}
	}


	// インデックスデータ作成
	for (const auto& mi : indices)
	{
		for (auto& index : mi)
		{
			m_indices.emplace_back(index);
		}
	}

	// サブセットデータ作成
	for (const auto& sub : subsets)
	{
		SUBSET subset{};
		subset.VertexBase = sub.VertexBase;
		subset.VertexNum = sub.VertexNum;
		subset.IndexBase = sub.IndexBase;
		subset.IndexNum = sub.IndexNum;
		subset.MtrlName = sub.mtrlname;
		subset.MaterialIdx = sub.materialindex;					//	マテリアル配列のインデックス
		m_subsets.emplace_back(subset);
	}

	// マテリアルデータ作成(表示のための)
	for (const auto& m : materials)
	{
		MATERIAL material{};
		material.Ambient = Color(m.Ambient.r, m.Ambient.g, m.Ambient.b, m.Ambient.a);
		material.Diffuse = Color(m.Diffuse.r, m.Diffuse.g, m.Diffuse.b, m.Diffuse.a);
		material.Specular = Color(m.Specular.r, m.Specular.g, m.Specular.b, m.Specular.a);
		material.Emission = Color(m.Emission.r, m.Emission.g, m.Emission.b, m.Emission.a);
		material.Shiness = m.Shiness;
		if (m.diffusetexturename.empty())
		{
			material.TextureEnable = FALSE;
			m_diffusetexturenames.emplace_back("");
		}
		else
		{
			material.TextureEnable = TRUE;
			m_diffusetexturenames.emplace_back(m.diffusetexturename);
		}

		m_materials.emplace_back(material);
	}
}