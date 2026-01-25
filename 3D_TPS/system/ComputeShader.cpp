#include "ComputeShader.h"
#include "renderer.h"
#include "dx11helper.h"

void ComputeShader::Create(const std::string& csPath)
{
    ID3D11Device* device = Renderer::GetDevice();

    bool ok = CreateComputeShader(
        device,
        csPath.c_str(),
        "main",
        "cs_5_0",
        &m_pComputeShader
    );

    if (!ok)
    {
        MessageBox(nullptr, "CreateComputeShader error", "error", MB_OK);
        return;
    }
}

void ComputeShader::SetGPU() const
{
    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();
    ctx->CSSetShader(m_pComputeShader.Get(), nullptr, 0);
}
