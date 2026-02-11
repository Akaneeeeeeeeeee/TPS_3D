#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class SpotShadowMapArray
{
public:
    bool Create(ID3D11Device* dev, int w, int h, int slices);
    void BeginSlice(ID3D11DeviceContext* ctx, int slice);
    void End(ID3D11DeviceContext* ctx) { (void)ctx; }
	void Release(void);

    ID3D11ShaderResourceView* GetSRV() const { return m_SRV.Get(); }
    int W() const { return m_W; }
    int H() const { return m_H; }
    int Slices() const { return m_Slices; }

private:
    int m_W = 0, m_H = 0, m_Slices = 0;
    ComPtr<ID3D11Texture2D> m_Tex;
    ComPtr<ID3D11ShaderResourceView> m_SRV;
    std::vector<ComPtr<ID3D11DepthStencilView>> m_DSV;
    D3D11_VIEWPORT m_VP{};
};
