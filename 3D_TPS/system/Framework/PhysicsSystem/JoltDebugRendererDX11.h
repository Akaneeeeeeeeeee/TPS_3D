#pragma once

#include "Framework/PhysicsSystem/Physics.h"
#include <mutex>
#include <string_view>
#include <d3d11.h>
#include "system/LineDrawer.h"
#include "commontypes.h"

struct LineInstanceParam;

//=========================
// JPH_DEBUG_RENDERER 有効時
//=========================
#ifdef JPH_DEBUG_RENDERER

#include <Jolt/Renderer/DebugRendererSimple.h>

class JoltDebugRendererDX11 final : public JPH::DebugRendererSimple
{
public:
    JoltDebugRendererDX11() = default;
    ~JoltDebugRendererDX11() override = default;

    void Begin(const DirectX::XMMATRIX& vp);
    void End();

    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawText3D(JPH::RVec3Arg inPosition,
        const std::string_view& inString,
        JPH::ColorArg inColor,
        float inHeight) override;

    struct LineVertex {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };

    struct StateGuard
    {
        ID3D11DeviceContext* ctx{};
        D3D11_PRIMITIVE_TOPOLOGY topo{};
        Microsoft::WRL::ComPtr<ID3D11InputLayout> il;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> gs;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vs_cb0;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rs;
        Microsoft::WRL::ComPtr<ID3D11BlendState> bs;
        FLOAT blendFactor[4]{};
        UINT sampleMask{};
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> dss;
        UINT stencilRef{};

        explicit StateGuard(ID3D11DeviceContext* c);
        ~StateGuard();
    };

private:
    void EnsureResources();

    Microsoft::WRL::ComPtr<ID3D11VertexShader>   mVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>    mPS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    mIL;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         mVB;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         mCB;   // VP行列用CB

    size_t mVBCapacity = 0;
    DirectX::XMMATRIX mVP{};
    bool mInitialized = false;

    std::vector<LineInstanceParam> m_Lines;
};

#else   //=========================
        // JPH_DEBUG_RENDERER 無効時
        //=========================

// Release 用のダミー実装
class JoltDebugRendererDX11 final
{
public:
    void Begin(const DirectX::XMMATRIX&) {}
    void End() {}
};

#endif // JPH_DEBUG_RENDERER
