#include "IShader.h"
#include "Src/Framework/Graphics/RenderManager.h"
#include "Src/Framework/Texture/LowLevel/Texture.h"

IShader::IShader(ShaderStage _Kind, const std::string& _Name) : m_Kind(_Kind), m_Name(_Name)
{
}

IShader::~IShader()
{
}

/// <summary>
/// �e�V�F�[�_�[�̃��t���N�V�������쐬���A�萔�o�b�t�@�ƃe�N�X�`���̈�𒲐�����
/// </summary>
/// <param name="pData"></param>
/// <param name="size"></param>
/// <returns></returns>
HRESULT IShader::Analyze_to_Adjust(void* pData, UINT size)
{
	HRESULT hr;
	ID3D11Device* pDevice = RenderManager::GetInstance().GetGraphicsDevice()->GetDevice();

	// ��͗p�̃��t���N�V�����쐬
	ID3D11ShaderReflection* pReflection;
	hr = D3DReflect(pData, size, IID_PPV_ARGS(&pReflection));
	if (FAILED(hr)) { return hr; }

	// �萔�o�b�t�@�쐬
	D3D11_SHADER_DESC shaderDesc;
	pReflection->GetDesc(&shaderDesc);
	m_pCBuffers.resize(shaderDesc.ConstantBuffers, nullptr);
	for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		// �V�F�[�_�[�̒萔�o�b�t�@�̏����擾
		D3D11_SHADER_BUFFER_DESC shaderBufDesc;
		ID3D11ShaderReflectionConstantBuffer* cbuf = pReflection->GetConstantBufferByIndex(i);
		cbuf->GetDesc(&shaderBufDesc);
		//// ��������f�o�b�O�p
		//// �X���b�g�ԍ��擾�i�� bufferDesc.Name ���g���j
		//D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		//hr = pReflection->GetResourceBindingDescByName(shaderBufDesc.Name, &bindDesc);
		//UINT slot = (SUCCEEDED(hr)) ? bindDesc.BindPoint : UINT_MAX;

		//const char* stageStr =
		//	stage == ShaderStage::Vertex ? "Vertex Shader" :
		//	stage == ShaderStage::Pixel ? "Pixel Shader" :
		//	stage == ShaderStage::Geometry ? "Geometry Shader" :
		//	stage == ShaderStage::Hull ? "Hull Shader" :
		//	stage == ShaderStage::Domain ? "Domain Shader" :
		//	stage == ShaderStage::Compute ? "Compute Shader" : "Unknown Shader";

		//std::cout << "==== Reflecting " << stageStr << " ====" << std::endl;
		//std::cout << "Constant Buffer: " << shaderBufDesc.Name << std::endl;
		//std::cout << "  Slot: b" << ((slot != UINT_MAX) ? std::to_string(slot) : "???") << std::endl;
		//std::cout << "  Size: " << shaderBufDesc.Size << " bytes" << std::endl;
		//std::cout << "  Variables: " << shaderBufDesc.Variables << std::endl;

		//// �ϐ��̏ڍׂ��擾
		//for (UINT v = 0; v < shaderBufDesc.Variables; v++)
		//{
		//	ID3D11ShaderReflectionVariable* pVariable = pReflection->GetConstantBufferByIndex(i)->GetVariableByIndex(v);
		//	D3D11_SHADER_VARIABLE_DESC varDesc;
		//	pVariable->GetDesc(&varDesc);

		//	std::cout << "    Variable #" << v << std::endl;
		//	std::cout << "      Name: " << varDesc.Name << std::endl;
		//	std::cout << "      StartOffset: " << varDesc.StartOffset << std::endl;
		//	std::cout << "      Size: " << varDesc.Size << " bytes" << std::endl;
		//}
		// �����܂Ńf�o�b�O�p
		// �쐬����o�b�t�@�̏��
		D3D11_BUFFER_DESC bufDesc = {};
		bufDesc.ByteWidth = shaderBufDesc.Size;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		// �o�b�t�@�̍쐬
		hr = pDevice->CreateBuffer(&bufDesc, nullptr, &m_pCBuffers[i]);
		if (FAILED(hr)) { return hr; }
	}
	// �e�N�X�`���̈�쐬
	m_pSRVs.resize(shaderDesc.BoundResources, nullptr);

	return Create(pData, size);
}

void IShader::WriteCBuffer(UINT _slot, const void* _pData)
{
	if (_slot < m_pCBuffers.size())
	{
		auto context = RenderManager::GetInstance().GetGraphicsDevice()->GetContext();
		context->UpdateSubresource(m_pCBuffers[_slot].Get(), 0, nullptr, _pData, 0, 0);
		//RenderManager::GetInstance().GetGraphicsDevice()->GetContext()->
		//	UpdateSubresource(m_pCBuffers[_slot].Get(), 0, nullptr, _pData, 0, 0);
	}
}

void IShader::SetTexture(UINT slot, Texture_Low* tex)
{
	if (slot >= m_pSRVs.size()) { return; }
	ID3D11ShaderResourceView* pTex = tex ? tex->GetResource() : nullptr;
	m_pSRVs[slot] = pTex;
	auto srv = m_pSRVs[slot].Get();
	auto context = RenderManager::GetInstance().GetGraphicsDevice()->GetContext();
	switch (m_Kind)
	{
		using enum ShaderStage;
	case Vertex:	context->VSSetShaderResources(slot, 1, &srv); break;
	case Pixel:		context->PSSetShaderResources(slot, 1, &srv); break;
	case Hull:		context->HSSetShaderResources(slot, 1, &srv); break;
	case Domain:	context->DSSetShaderResources(slot, 1, &srv); break;
	default:		break;
	}
}
