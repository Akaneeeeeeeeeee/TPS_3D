#pragma once

#include "Framework/PhysicsSystem/Physics.h"

#include <Jolt/Renderer/DebugRendererSimple.h>

#include <mutex>
#include <string_view>
#include <d3d11.h>
#include "commontypes.h"

class JoltDebugRendererDX11 final : public JPH::DebugRendererSimple
{
public:
    JoltDebugRendererDX11() = default;
    ~JoltDebugRendererDX11() override = default;

    void Begin(const DirectX::XMMATRIX& vp);
    void End();

    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override;

    struct LineVertex {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
    };


    // 最低限のパイプラインを保存/復元するガード
    struct StateGuard
    {
        ID3D11DeviceContext* ctx{};
        D3D11_PRIMITIVE_TOPOLOGY topo{};
        ComPtr<ID3D11InputLayout> il;
        ComPtr<ID3D11VertexShader> vs;
        ComPtr<ID3D11PixelShader> ps;
        ComPtr<ID3D11GeometryShader> gs;
        ComPtr<ID3D11Buffer> vs_cb0;
        ComPtr<ID3D11RasterizerState> rs;
        ComPtr<ID3D11BlendState> bs;
        FLOAT blendFactor[4]{};
        UINT sampleMask{};
        ComPtr<ID3D11DepthStencilState> dss;
        UINT stencilRef{};

        explicit StateGuard(ID3D11DeviceContext* c) : ctx(c)
        {
            ctx->IAGetPrimitiveTopology(&topo);
            ctx->IAGetInputLayout(&il);

            ctx->VSGetShader(&vs, nullptr, nullptr);
            ctx->PSGetShader(&ps, nullptr, nullptr);
            ctx->GSGetShader(&gs, nullptr, nullptr);

            ID3D11Buffer* tcb = nullptr;
            ctx->VSGetConstantBuffers(0, 1, &tcb);
            vs_cb0.Attach(tcb);

            ctx->RSGetState(&rs);
            ctx->OMGetBlendState(&bs, blendFactor, &sampleMask);
            ctx->OMGetDepthStencilState(&dss, &stencilRef);
        }

        ~StateGuard()
        {
            ctx->IASetPrimitiveTopology(topo);
            ctx->IASetInputLayout(il.Get());

            ctx->VSSetShader(vs.Get(), nullptr, 0);
            ID3D11Buffer* cb = vs_cb0.Get();
            ctx->VSSetConstantBuffers(0, 1, &cb);

            ctx->PSSetShader(ps.Get(), nullptr, 0);
            ctx->GSSetShader(gs.Get(), nullptr, 0);

            ctx->RSSetState(rs.Get());
            ctx->OMSetBlendState(bs.Get(), blendFactor, sampleMask);
            ctx->OMSetDepthStencilState(dss.Get(), stencilRef);
        }
    };


private:
    void FlushLines();
    void EnsureResources();

    Microsoft::WRL::ComPtr<ID3D11VertexShader>   mVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>    mPS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>    mIL;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         mVB;
    Microsoft::WRL::ComPtr<ID3D11Buffer>         mCB;   // VP行列用CB

    size_t mVBCapacity = 0;
    DirectX::XMMATRIX mVP{};
    bool mInitialized = false;

	std::vector<LineVertex> m_Lines;
};

