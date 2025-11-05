#include "PixelShader.h"
//#include "system/Framework/Graphics/RenderManager.h"
#include "system/renderer.h"

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
	ID3D11Device* device = Renderer::GetDevice();
	HRESULT hr = device->CreatePixelShader(pData, size, nullptr, &m_pPixelShader);
	if (FAILED(hr)) return hr;

	// Reflection は親が持つ（必要なら Analyze_to_Adjust を呼ぶ）
	return S_OK;
}

void PixelShader::Bind(const RenderInfo& info)
{
	ID3D11DeviceContext* pContext = Renderer::GetDeviceContext();
	pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
    //// cBuffers をループしてセット
    //for (const auto& cb : info.cBuffers)
    //{
    //    for (const auto& refCb : m_ShaderReflection.cbuffers)
    //    {
    //        if (cb.name == refCb.name)
    //            pContext->PSSetConstantBuffers(refCb.slot, 1, &cb.buffer);
    //    }
    //}

    //// SRV バインド
    //for (const auto& srv : info.srvs)
    //{
    //    for (const auto& refSrv : m_ShaderReflection.srvs)
    //    {
    //        if (srv.name == refSrv.name)
    //            pContext->PSSetShaderResources(refSrv.slot, 1, &srv.view);
    //    }
    //}
}

void PixelShader::Unbind(void)
{
	Renderer::GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);
}