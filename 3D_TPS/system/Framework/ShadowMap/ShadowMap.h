#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class ShadowMap
{
public:
    bool Create(ID3D11Device* dev, int w, int h);
    void Begin(ID3D11DeviceContext* ctx);
    void End(ID3D11DeviceContext* ctx);

    ID3D11ShaderResourceView* GetSRV() const { return m_SRV.Get(); }
    ID3D11DepthStencilView* GetDSV() const { return m_DSV.Get(); }

    int GetW() const { return m_W; }
    int GetH() const { return m_H; }

private:
    int m_W = 0, m_H = 0;
    ComPtr<ID3D11Texture2D> m_Tex;
    ComPtr<ID3D11DepthStencilView> m_DSV;
    ComPtr<ID3D11ShaderResourceView> m_SRV;

    D3D11_VIEWPORT m_VP{};
};
