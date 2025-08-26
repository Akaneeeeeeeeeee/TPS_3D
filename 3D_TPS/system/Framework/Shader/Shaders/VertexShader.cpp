#include "VertexShader.h"
#include "Src/Framework/Graphics/RenderManager.h"

/// <summary>
/// �^�̓R���X�g���N�^�Ŏ����Őݒ�
/// </summary>
VertexShader::VertexShader(const std::string& _name)
	: IShader(ShaderStage::Vertex, _name), m_pInputLayout(nullptr)
{
}

VertexShader::~VertexShader()
{
}

HRESULT VertexShader::Create(void* pData, UINT size)
{
	HRESULT hr;
	// �f�o�C�X�擾
	ID3D11Device* device = RenderManager::GetInstance().GetGraphicsDevice()->GetDevice();

	// �V�F�[�_�[�쐬
	hr = device->CreateVertexShader(pData, size, NULL, &m_pVertexShader);
	if (FAILED(hr)) { return hr; }

	/*
	�V�F�[�_�쐬���ɃV�F�[�_���t���N�V������ʂ��ăC���v�b�g���C�A�E�g���擾
	�Z�}���e�B�N�X�̔z�u�Ȃǂ��环�ʎq���쐬
	���ʎq���o�^�ρ��ė��p�A�Ȃ���ΐV�K�쐬
	https://blog.techlab-xe.net/dxc-shader-reflection/
	*/

	// ���t���N�V�����p
	ComPtr<ID3D11ShaderReflection> pReflection;
	hr = D3DReflect(pData, size, IID_PPV_ARGS(pReflection.GetAddressOf()));
	if (FAILED(hr)) return hr;

	D3D11_SHADER_DESC shaderDesc = {};
	pReflection->GetDesc(&shaderDesc);

	// ���͗v�f�z��ivector�Ŏ�������j
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputDescs(shaderDesc.InputParameters);
	DXGI_FORMAT formats[][4] =
	{
		{
			DXGI_FORMAT_R32_UINT,
			DXGI_FORMAT_R32G32_UINT,
			DXGI_FORMAT_R32G32B32_UINT,
			DXGI_FORMAT_R32G32B32A32_UINT,
		}, {
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32G32_SINT,
			DXGI_FORMAT_R32G32B32_SINT,
			DXGI_FORMAT_R32G32B32A32_SINT,
		}, {
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
		},
	};


	for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
	{
		D3D11_SIGNATURE_PARAMETER_DESC sigDesc = {};
		pReflection->GetInputParameterDesc(i, &sigDesc);

		auto& desc = inputDescs[i];
		desc.SemanticName = sigDesc.SemanticName;
		desc.SemanticIndex = sigDesc.SemanticIndex;

		// �}�X�N����v�f�����v�Z
		// http://marupeke296.com/TIPS_No17_Bit.html
		BYTE elementCount = sigDesc.Mask;
		elementCount = (elementCount & 0x05) + ((elementCount >> 1) & 0x05);
		elementCount = (elementCount & 0x03) + ((elementCount >> 2) & 0x03);

		// �f�[�^�^���Ƃ�DXGI_FORMAT��I��
        switch (sigDesc.ComponentType)
        {
        case D3D_REGISTER_COMPONENT_UINT32:
            desc.Format = formats[0][elementCount - 1];
            break;
        case D3D_REGISTER_COMPONENT_SINT32:
            desc.Format = formats[1][elementCount - 1];
            break;
        case D3D_REGISTER_COMPONENT_FLOAT32:
            desc.Format = formats[2][elementCount - 1];
            break;
        default:
            desc.Format = DXGI_FORMAT_UNKNOWN;
            break;
        }

		desc.InputSlot = 0;
		desc.AlignedByteOffset = (i == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;
	}

	// ���̓��C�A�E�g�쐬
	hr = device->CreateInputLayout(
		inputDescs.data(),
		static_cast<UINT>(inputDescs.size()),
		pData,
		size,
		m_pInputLayout.GetAddressOf()
	);

	return hr;
}

void VertexShader::Bind(void)
{
	ID3D11DeviceContext* pContext = RenderManager::GetInstance().GetGraphicsDevice()->GetContext();
	pContext->VSSetShader(m_pVertexShader.Get(), NULL, 0);
	pContext->IASetInputLayout(m_pInputLayout.Get());
	for (int i = 0; i < m_pCBuffers.size(); ++i)
	{
		//pContext->VSSetConstantBuffers(i, 1, &m_pCBuffers[i]);
		auto* buf = m_pCBuffers[i].Get();
		pContext->VSSetConstantBuffers(i, 1, &buf);
	}
	for (int i = 0; i < m_pSRVs.size(); ++i)
	{
		auto* srv = m_pSRVs[i].Get();
		pContext->VSSetShaderResources(i, 1, &srv);
		//pContext->VSSetShaderResources(i, 1, &m_pSRVs[i]);
	}
}

void VertexShader::Unbind(void)
{
	RenderManager::GetInstance().GetGraphicsDevice()->GetContext()->VSSetShader(nullptr, nullptr, 0);
}