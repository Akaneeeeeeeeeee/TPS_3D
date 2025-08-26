#pragma once
#include "system/Framework/Shader/IShader/IShader.h"
#include <unordered_map>
#include <memory>

static constexpr const char* TargetList[]
{
	"vs_5_0",
	"gs_5_0",
	"hs_5_0",
	"ds_5_0",
	"ps_5_0"
};

static constexpr const char* EntryPoint = "main"; //! �e�V�F�[�_�[�̃G���g���[�|�C���g��

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	//! �O���̏����������Ŏg�������V�F�[�_�[�����ׂ�Load()���A�o�^���Ă���
	//! m_ShaderManager->Load(ShaderStage::Vertex, Assets/Shader/VS_PBR.cso, "VS_PBR");�݂����Ȋ���
	void Init(void);	//! ����������

	HRESULT Load(ShaderStage _Stage, const std::filesystem::path& _csoPath, const std::string& _name);

	IShader* GetShader(const std::string& _name) const;	//! ���O�ŃV�F�[�_�[���擾

private:
	// ���O�œo�^��������VS_PBR�Ƃ�PS_PBR�Ƃ�
	// ����������cso��ǂݍ��݁A���O�œo�^����
	// ���e�V�F�[�_�[�I�u�W�F�N�g�͎��g�̖��O�iVS_PBR�Ƃ��j�������A���������̓t�@�C���p�X�������œn��
	std::unordered_map<std::string, std::unique_ptr<IShader>> m_Shaders; //! �V�F�[�_�[�̃R���e�i
};

