#pragma once

#include	<vector>
#include	<wrl/client.h>
#include	"dx11helper.h"
#include	"NonCopyable.h"

#include	"renderer.h"

using Microsoft::WRL::ComPtr;

class MyIndexBuffer : NonCopyable {

	ComPtr<ID3D11Buffer> m_IndexBuffer;
	DXGI_FORMAT m_IndexFormat = DXGI_FORMAT_R32_UINT;

public:
	void Create(const std::vector<unsigned int>& indices) {

		// デバイス取得
		ID3D11Device* device = nullptr;

		device = Renderer::GetDevice();
		m_IndexFormat = DXGI_FORMAT_R32_UINT;

		assert(device);

		// インデックスバッファ作成
		bool sts = CreateIndexBuffer(
			device,										// デバイス
			(unsigned int)(indices.size()),				// インデックス数
			(void*)indices.data(),						// インデックスデータ先頭アドレス
			&m_IndexBuffer);							// インデックスバッファ

		assert(sts == true);
	}

	void SetGPU() {
		// デバイスコンテキスト取得
		ID3D11DeviceContext* devicecontext = nullptr;
		devicecontext = Renderer::GetDeviceContext();

		// インデックスバッファをセット
		devicecontext->IASetIndexBuffer(m_IndexBuffer.Get(), m_IndexFormat, 0);

	}

	// インデックスバッファを書き換える
	void Modify(const std::vector<unsigned int>& indices)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		HRESULT hr = Renderer::GetDeviceContext()->Map(
			m_IndexBuffer.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD, 0, &msr);

		if (SUCCEEDED(hr)) {
			memcpy(msr.pData, indices.data(), indices.size() * sizeof(unsigned int));
			Renderer::GetDeviceContext()->Unmap(m_IndexBuffer.Get(), 0);
		}
	}

	ID3D11Buffer* GetBuffer(void)
	{
		return m_IndexBuffer.Get();
	}

	UINT GetIndexCount(void)
	{
		D3D11_BUFFER_DESC descriptor;
		m_IndexBuffer->GetDesc(&descriptor);
		return descriptor.ByteWidth / sizeof(unsigned int);
	}

	DXGI_FORMAT GetIndexFormat(void)
	{
		return m_IndexFormat;
	}
};
