#pragma once
#include <string>
#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

class ComputeShader
{
public:
    void Create(const std::string& csPath); // HLSL‚ğƒRƒ“ƒpƒCƒ‹‚µ‚ÄCS¶¬
    void SetGPU() const;                   // CSSetShader

private:
    ComPtr<ID3D11ComputeShader> m_pComputeShader;
};
