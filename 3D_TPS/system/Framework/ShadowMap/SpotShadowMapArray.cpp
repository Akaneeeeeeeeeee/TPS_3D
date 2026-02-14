#include "SpotShadowMapArray.h"


bool SpotShadowMapArray::Create(ID3D11Device* dev, int w, int h, int slices)
{
    m_W = w; m_H = h; m_Slices = slices;

    D3D11_TEXTURE2D_DESC td{};
    td.Width = w; td.Height = h;
    td.MipLevels = 1;
    td.ArraySize = slices;
    td.Format = DXGI_FORMAT_R32_TYPELESS;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(dev->CreateTexture2D(&td, nullptr, m_Tex.GetAddressOf()))) return false;

    m_DSV.resize(slices);
    for (int i = 0; i < slices; i++)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
        dsvd.Format = DXGI_FORMAT_D32_FLOAT;
        dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvd.Texture2DArray.MipSlice = 0;
        dsvd.Texture2DArray.FirstArraySlice = i;
        dsvd.Texture2DArray.ArraySize = 1;

        if (FAILED(dev->CreateDepthStencilView(m_Tex.Get(), &dsvd, m_DSV[i].GetAddressOf())))
            return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.Format = DXGI_FORMAT_R32_FLOAT;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvd.Texture2DArray.MostDetailedMip = 0;
    srvd.Texture2DArray.MipLevels = 1;
    srvd.Texture2DArray.FirstArraySlice = 0;
    srvd.Texture2DArray.ArraySize = slices;

    if (FAILED(dev->CreateShaderResourceView(m_Tex.Get(), &srvd, m_SRV.GetAddressOf())))
        return false;

    m_VP = { 0,0,(float)w,(float)h,0,1 };
    return true;
}

void SpotShadowMapArray::BeginSlice(ID3D11DeviceContext* ctx, int slice)
{
    ctx->OMSetRenderTargets(0, nullptr, m_DSV[slice].Get());
    ctx->RSSetViewports(1, &m_VP);
    ctx->ClearDepthStencilView(m_DSV[slice].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
