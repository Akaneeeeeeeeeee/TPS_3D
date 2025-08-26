#include "PixelShader.h"
#include "Src/Framework/Graphics/RenderManager.h"

/// <summary>
/// 型はコンストラクタで自動で設定
/// </summary>
PixelShader::PixelShader(const std::string& _name)
	: IShader(ShaderStage::Pixel, _name), m_pPixelShader(nullptr)
{
}

PixelShader::~PixelShader()
{
}


HRESULT PixelShader::Create(void* pData, UINT size)
{
	ID3D11Device* device = RenderManager::GetInstance().GetGraphicsDevice()->GetDevice();
	return device->CreatePixelShader(pData, size, NULL, &m_pPixelShader);
}

void PixelShader::Bind(void)
{
	ID3D11DeviceContext* pContext = RenderManager::GetInstance().GetGraphicsDevice()->GetContext();
	pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	for (int i = 0; i < m_pCBuffers.size(); ++i)
	{
		auto* buf = m_pCBuffers[i].Get();
		pContext->PSSetConstantBuffers(i, 1, &buf);
		//pContext->PSSetConstantBuffers(i, 1, &m_pCBuffers[i]);
	}
	for (int i = 0; i < m_pSRVs.size(); ++i)
	{
		auto* srv = m_pSRVs[i].Get();
		pContext->PSSetShaderResources(i, 1, &srv);
		//pContext->PSSetShaderResources(i, 1, &m_pSRVs[i]);
	}
}

void PixelShader::Unbind(void)
{
	RenderManager::GetInstance().GetGraphicsDevice()->GetContext()->PSSetShader(nullptr, nullptr, 0);
}