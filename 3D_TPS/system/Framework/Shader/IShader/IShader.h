#pragma once
#include <d3d11.h>
#include <vector>
#include <wrl/client.h>
#include <filesystem>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


using namespace Microsoft::WRL;

class Texture_Low; // �O���錾

//! �e�V�F�[�_�[�̎��ʗp
enum class ShaderStage {
	Vertex,
	Pixel,
	Geometry,
	Compute,
	Hull,
	Domain,

	Stage_MAX
};

class IShader
{
public:
	virtual ~IShader();

	// �V�F�[�_�[�t�@�C����ǂݍ��񂾌�A�V�F�[�_�[�̎�ޕʂɏ������s��
	HRESULT Analyze_to_Adjust(void* pData, UINT size);

	virtual void Bind() = 0;	// �e�V�F�[�_�[�̊��蓖�ėp�������z�֐�
	virtual void Unbind() = 0;	// �e�V�F�[�_�[�̊��蓖�ĉ����p�������z�֐�
	virtual void WriteCBuffer(UINT _Slot, const void* _pData);
	virtual void SetTexture(UINT slot, Texture_Low* tex);


	//virtual void SetSRV(UINT _Slot, ID3D11ShaderResourceView* _pSRV) = 0;	// �V�F�[�_�[���\�[�X�r���[���Z�b�g

	virtual std::string GetName() const { return m_Name; }	// �V�F�[�_�[�̖��O���擾

protected:
	// �V�F�[�_�[�̍쐬(�e�V�F�[�_�[�Ŏ���)
	virtual HRESULT Create(void* pData, UINT size) = 0;

	IShader(ShaderStage _Kind, const std::string& _Name);
	ShaderStage m_Kind;		// �V�F�[�_�[�̎�ށi���_�A�s�N�Z���A�W�I���g���Ȃǁj
	std::string m_Name;		// �V�F�[�_�[�̖��O�i�t�@�C�����Ȃǁj
	std::vector<ComPtr<ID3D11Buffer>> m_pCBuffers;			// �萔�o�b�t�@�̔z��
	std::vector<ComPtr<ID3D11ShaderResourceView>> m_pSRVs;	// �V�F�[�_�[���\�[�X�r���[�̔z��
};

