#pragma once
#include "Src/Framework/Shader/IShader/IShader.h"


class PixelShader final : public IShader
{
public:
	PixelShader(const std::string& _name);
	~PixelShader();

	void Bind(void) override;
	void Unbind(void) override;
	HRESULT Create(void* pData, UINT size) override;

private:
	ComPtr<ID3D11PixelShader> m_pPixelShader;
};

