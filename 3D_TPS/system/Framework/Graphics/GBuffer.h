#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <array>

using Microsoft::WRL::ComPtr;

class GBuffer
{
public:
    static constexpr int RT_COUNT = 3;

    bool Create(ID3D11Device* dev, int w, int h);
    void Release();
    bool Resize(ID3D11Device* dev, int w, int h);

    void Begin(ID3D11DeviceContext* ctx, ID3D11DepthStencilView* dsv);
    void End(ID3D11DeviceContext* ctx);

    ID3D11ShaderResourceView* GetSRV(int i) const { return m_SRV[i].Get(); }
    ID3D11RenderTargetView* GetRTV(int i) const { return m_RTV[i].Get(); }

    int GetW() const { return m_W; }
    int GetH() const { return m_H; }

private:
    bool CreateInternal(ID3D11Device* dev, int w, int h);

private:
    int m_W = 0, m_H = 0;

    std::array<ComPtr<ID3D11Texture2D>, RT_COUNT> m_Tex{};
    std::array<ComPtr<ID3D11RenderTargetView>, RT_COUNT> m_RTV{};
    std::array<ComPtr<ID3D11ShaderResourceView>, RT_COUNT> m_SRV{};
};
