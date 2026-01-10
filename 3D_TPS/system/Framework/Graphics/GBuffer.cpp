#include "GBuffer.h"

static void ClearRT(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv)
{
    float c[4] = { 0,0,0,0 };
    ctx->ClearRenderTargetView(rtv, c);
}

bool GBuffer::Create(ID3D11Device* dev, int w, int h)
{
    Release();
    return CreateInternal(dev, w, h);
}

void GBuffer::Release()
{
    for (auto& x : m_SRV) x.Reset();
    for (auto& x : m_RTV) x.Reset();
    for (auto& x : m_Tex) x.Reset();
    m_W = m_H = 0;
}

bool GBuffer::Resize(ID3D11Device* dev, int w, int h)
{
    if (w == m_W && h == m_H) return true;
    return Create(dev, w, h);
}

bool GBuffer::CreateInternal(ID3D11Device* dev, int w, int h)
{
    m_W = w; m_H = h;

    // RT0: Albedo (8bit)
    // RT1: Normal+Roughness (16F)
    // RT2: Emission+Spec (16F)
    DXGI_FORMAT fmts[RT_COUNT] = {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R16G16B16A16_FLOAT
    };

    for (int i = 0; i < RT_COUNT; i++)
    {
        D3D11_TEXTURE2D_DESC td{};
        td.Width = w;
        td.Height = h;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = fmts[i];
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        if (FAILED(dev->CreateTexture2D(&td, nullptr, m_Tex[i].GetAddressOf()))) return false;
        if (FAILED(dev->CreateRenderTargetView(m_Tex[i].Get(), nullptr, m_RTV[i].GetAddressOf()))) return false;
        if (FAILED(dev->CreateShaderResourceView(m_Tex[i].Get(), nullptr, m_SRV[i].GetAddressOf()))) return false;
    }
    return true;
}

void GBuffer::Begin(ID3D11DeviceContext* ctx, ID3D11DepthStencilView* dsv)
{
    ID3D11RenderTargetView* rtvs[RT_COUNT] = { m_RTV[0].Get(), m_RTV[1].Get(), m_RTV[2].Get() };
    ctx->OMSetRenderTargets(RT_COUNT, rtvs, dsv);

    D3D11_VIEWPORT vp{};
    vp.Width = (float)m_W;
    vp.Height = (float)m_H;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    ctx->RSSetViewports(1, &vp);

    for (int i = 0; i < RT_COUNT; i++) ClearRT(ctx, m_RTV[i].Get());
}

void GBuffer::End(ID3D11DeviceContext* ctx)
{
    // 何もしない（必要ならここでRTを戻す）
}
