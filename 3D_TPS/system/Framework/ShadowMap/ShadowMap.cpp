#include "ShadowMap.h"

bool ShadowMap::Create(ID3D11Device* dev, int w, int h)
{
    m_W = w; m_H = h;

    // typeless depth
    D3D11_TEXTURE2D_DESC td{};
    td.Width = (UINT)w;
    td.Height = (UINT)h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R32_TYPELESS;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = dev->CreateTexture2D(&td, nullptr, m_Tex.GetAddressOf());
    if (FAILED(hr)) return false;

    // DSV
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
    dsvd.Format = DXGI_FORMAT_D32_FLOAT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;
    hr = dev->CreateDepthStencilView(m_Tex.Get(), &dsvd, m_DSV.GetAddressOf());
    if (FAILED(hr)) return false;

    // SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.Format = DXGI_FORMAT_R32_FLOAT;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    srvd.Texture2D.MostDetailedMip = 0;
    hr = dev->CreateShaderResourceView(m_Tex.Get(), &srvd, m_SRV.GetAddressOf());
    if (FAILED(hr)) return false;

    // viewport
    m_VP.TopLeftX = 0;
    m_VP.TopLeftY = 0;
    m_VP.Width = (float)w;
    m_VP.Height = (float)h;
    m_VP.MinDepth = 0.0f;
    m_VP.MaxDepth = 1.0f;

    return true;
}

void ShadowMap::Begin(ID3D11DeviceContext* ctx)
{
    ctx->OMSetRenderTargets(0, nullptr, m_DSV.Get());
    ctx->RSSetViewports(1, &m_VP);

    ctx->ClearDepthStencilView(m_DSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMap::End(ID3D11DeviceContext* ctx)
{
    // Ç±Ç±Ç≈ÇÕâΩÇ‡ÇµÇ»Ç¢ÅiåƒÇ—èoÇµë§Ç≈ backbuffer Ç…ñﬂÇ∑Åj
    (void)ctx;
}
