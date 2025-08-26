#pragma once
#include "Mesh.h"
#include "Src/Framework/Texture/LowLevel/Texture.h"

/// <summary>
/// �ÓI�ȃ��b�V������舵���N���X
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

	std::vector<MATERIAL> m_materials;					// �}�e���A�����
	std::vector<std::string> m_diffusetexturenames;		// �f�B�t���[�Y�e�N�X�`����
	std::vector<SUBSET> m_subsets;						// �T�u�Z�b�g���

	std::vector<std::unique_ptr<Texture_Low>>	m_diffusetextures;	// �f�B�t���[�Y�e�N�X�`���Q
};