#pragma once
#include "Src/Framework/Shader/IShader/IShader.h"


class VertexShader final : public IShader
{
public:
	VertexShader(const std::string& _name);
	~VertexShader();

	void Bind(void) override;
	void Unbind(void) override;
	HRESULT Create(void* pData, UINT size) override;

private:
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11InputLayout> m_pInputLayout;
};

