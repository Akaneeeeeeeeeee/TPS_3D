#include	"StaticMesh.h"
#include	"Src/Framework/Assimp/AssimpPerse.h"

void StaticMesh::Load(std::string filename, std::string texturedirectory)
{
	std::vector<GM31::GE::myAssimp::SUBSET> subsets{};					// �T�u�Z�b�g���
	std::vector<std::vector<GM31::GE::myAssimp::VERTEX>> vertices{};	// ���_�f�[�^�i���b�V���P�ʁj
	std::vector<std::vector<unsigned int>> indices{};					// �C���f�b�N�X�f�[�^�i���b�V���P�ʁj
	std::vector<GM31::GE::myAssimp::MATERIAL> materials{};				// �}�e���A��
	std::vector<std::unique_ptr<Texture>> embededtextures{};			// �����e�N�X�`���Q

	// assimp���g�p���ă��f���f�[�^���擾
	GM31::GE::myAssimp::GetModelData(filename, texturedirectory);

	subsets = GM31::GE::myAssimp::GetSubsets();							// �T�u�Z�b�g���擾
	vertices = GM31::GE::myAssimp::GetVertices();						// ���_�f�[�^�i���b�V���P�ʁj
	indices = GM31::GE::myAssimp::GetIndices();							// �C���f�b�N�X�f�[�^�i���b�V���P�ʁj
	materials = GM31::GE::myAssimp::GetMaterials();						// �}�e���A�����擾

	m_diffusetextures = GM31::GE::myAssimp::GetDiffuseTextures();		// ���������������e�N�X�`�����擾	

	// ���_�f�[�^�쐬
	int meshidx = 0;

	for (const auto& mv : vertices)
	{
		for (auto& v : mv)
		{
			VERTEX_3D vertex{};
			vertex.Position = Vector3(v.pos.x, v.pos.y, v.pos.z);
			vertex.Normal = Vector3(v.normal.x, v.normal.y, v.normal.z);
			vertex.TexCoord = Vector2(v.texcoord.x, v.texcoord.y);
			vertex.Diffuse = Color(v.color.r, v.color.g, v.color.b, v.color.a);

			vertex.bonecnt = v.bonecnt;
			for (int i = 0; i < 4; i++)
			{
				vertex.BoneIndex[i] = 0;
				vertex.BoneWeight[i] = 0.0f;
				vertex.BoneName[i] = "";
			}

			for (int i = 0; i < v.bonecnt; i++)
			{
				vertex.BoneIndex[i] = v.BoneIndex[i];
				vertex.BoneWeight[i] = v.BoneWeight[i];
				vertex.BoneName[i] = v.BoneName[i];
			}

			m_vertices.emplace_back(vertex);
		}
	}

	// �C���f�b�N�X�f�[�^�쐬
	for (const auto& mi : indices)
	{
		for (auto& index : mi)
		{
			m_indices.emplace_back(index);
		}
	}

	// �T�u�Z�b�g�f�[�^�쐬
	for (const auto& sub : subsets)
	{
		SUBSET subset{};
		subset.VertexBase = sub.VertexBase;
		subset.VertexNum = sub.VertexNum;
		subset.IndexBase = sub.IndexBase;
		subset.IndexNum = sub.IndexNum;
		subset.MtrlName = sub.mtrlname;
		subset.MaterialIdx = sub.materialindex;					//	�}�e���A���z��̃C���f�b�N�X
		m_subsets.emplace_back(subset);
	}

	// �}�e���A���f�[�^�쐬(�\���̂��߂�)
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