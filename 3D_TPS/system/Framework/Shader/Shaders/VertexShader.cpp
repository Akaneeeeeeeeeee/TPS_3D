#include "VertexShader.h"
#include "Src/Framework/Graphics/RenderManager.h"

/// <summary>
/// 型はコンストラクタで自動で設定
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
	// デバイス取得
	ID3D11Device* device = RenderManager::GetInstance().GetGraphicsDevice()->GetDevice();

	// シェーダー作成
	hr = device->CreateVertexShader(pData, size, NULL, &m_pVertexShader);
	if (FAILED(hr)) { return hr; }

	/*
	シェーダ作成時にシェーダリフレクションを通してインプットレイアウトを取得
	セマンティクスの配置などから識別子を作成
	識別子が登録済→再利用、なければ新規作成
	https://blog.techlab-xe.net/dxc-shader-reflection/
	*/

	// リフレクション用
	ComPtr<ID3D11ShaderReflection> pReflection;
	hr = D3DReflect(pData, size, IID_PPV_ARGS(pReflection.GetAddressOf()));
	if (FAILED(hr)) return hr;

	D3D11_SHADER_DESC shaderDesc = {};
	pReflection->GetDesc(&shaderDesc);

	// 入力要素配列（vectorで自動解放）
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

		// マスクから要素数を計算
		// http://marupeke296.com/TIPS_No17_Bit.html
		BYTE elementCount = sigDesc.Mask;
		elementCount = (elementCount & 0x05) + ((elementCount >> 1) & 0x05);
		elementCount = (elementCount & 0x03) + ((elementCount >> 2) & 0x03);

		// データ型ごとにDXGI_FORMATを選択
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

	// 入力レイアウト作成
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