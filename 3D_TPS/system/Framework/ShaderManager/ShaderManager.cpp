#include "ShaderManager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <fstream>
#include "Src/Framework/Shader/Shaders/VertexShader.h"
#include "Src/Framework/Shader/Shaders/PixelShader.h"
#include "Src/Framework/Graphics/RenderManager.h"

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::Init()
{
    // �V�F�[�_�[�̏������������K�v�ȏꍇ�͂����ɋL�q
    // �V�F�[�_�[���ꊇ�Ń��[�h���ăL���b�V��
	this->Load(ShaderStage::Vertex, "Assets/Shader/unlitTextureVS.cso", "unlitTextureVS");
	this->Load(ShaderStage::Pixel, "Assets/Shader/unlitTexturePS.cso", "unlitTexturePS");
	this->Load(ShaderStage::Vertex, "Assets/Shader/DefaultVS.cso", "DefaultVS");
	this->Load(ShaderStage::Pixel, "Assets/Shader/DefaultPS.cso", "DefaultPS");
}

/// <summary>
/// �w�肳�ꂽ�V�F�[�_�[�X�e�[�W��CSO�t�@�C���p�X����V�F�[�_�[��ǂݍ��݂ƍ쐬���s��
/// </summary>
/// <param name="_stage">�ǂݍ��ރV�F�[�_�[�̃X�e�[�W�i��: Vertex, Pixel �Ȃǁj�B</param>
/// <param name="csoPath">CSO�t�@�C���̃p�X</param>
/// <returns>�V�F�[�_�[�̓ǂݍ��݂ƍ쐬�����������ꍇ�� S_OK�A���s�����ꍇ�� E_FAIL ��Ԃ�</returns>
HRESULT ShaderManager::Load(ShaderStage _stage, const std::filesystem::path& csoPath, const std::string& _name)
{
	// �t�@�C���p�X�ɑ��݂��Ȃ��ꍇ�̓G���[
    if (!std::filesystem::exists(csoPath)) { return E_FAIL; }

    // �o�C�i���ǂݍ���
    std::ifstream file(csoPath, std::ios::binary | std::ios::ate);
    
	// �t�@�C�����J���Ȃ������ꍇ�̓G���[
    if (!file) { return E_FAIL; }

	// �t�@�C���T�C�Y���擾
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

	// �t�@�C���T�C�Y��0�ȉ��̏ꍇ�̓G���[
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) { return E_FAIL; }

	std::unique_ptr<IShader> shader;

    switch (_stage)
    {
    case ShaderStage::Vertex:
		shader = std::make_unique<VertexShader>(_name);
        break;
    case ShaderStage::Pixel:
		shader = std::make_unique<PixelShader>(_name);
        break;
    case ShaderStage::Geometry:
        break;
    case ShaderStage::Compute:
        break;
    case ShaderStage::Hull:
        break;
    case ShaderStage::Domain:
        break;
    default:
        break;
    }

	// �����őS�̂̃��t���N�V�����쐬���萔�o�b�t�@��srv�̐ݒ���s���A�e�V�F�[�_�[�̍쐬���s��

    if (FAILED(shader->Analyze_to_Adjust(buffer.data(), static_cast<UINT>(size)))) {
		return E_FAIL;
    }

    m_Shaders[_name] = std::move(shader);
	return S_OK;
}


IShader* ShaderManager::GetShader(const std::string& _name) const
{
    auto it = m_Shaders.find(_name);
    if (it != m_Shaders.end()) {
        return it->second.get();
    }
    return nullptr;
}