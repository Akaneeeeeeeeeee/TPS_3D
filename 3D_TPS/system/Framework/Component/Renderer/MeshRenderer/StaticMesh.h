#pragma once
#include "Mesh.h"
#include "Src/Framework/Texture/LowLevel/Texture.h"

/// <summary>
/// 静的なメッシュを取り扱うクラス
/// </summary>
class StaticMesh : public Mesh {
public:
	void Load(std::string filename, std::string texturedirectory = "");

	const std::vector<MATERIAL>& GetMaterials() {
		return m_materials;
	}

	const std::vector<SUBSET>& GetSubsets() {
		return m_subsets;
	}

	const std::vector<std::string>& GetDiffuseTextureNames() {
		return m_diffusetexturenames;
	}

	std::vector<std::unique_ptr<Texture_Low>> GetDiffuseTextures() {
		return std::move(m_diffusetextures);
	}

private:

	std::vector<MATERIAL> m_materials;					// マテリアル情報
	std::vector<std::string> m_diffusetexturenames;		// ディフューズテクスチャ名
	std::vector<SUBSET> m_subsets;						// サブセット情報

	std::vector<std::unique_ptr<Texture_Low>>	m_diffusetextures;	// ディフューズテクスチャ群
};