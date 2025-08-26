#pragma once

#include	<vector>
#include	<wrl/client.h>
#include	"system/dx11helper.h"
#include	"system/Framework/Graphics/RenderManager.h"

using Microsoft::WRL::ComPtr;

//-----------------------------------------------------------------------------
//IndexBufferクラス
//-----------------------------------------------------------------------------
class IndexBuffer {

	ComPtr<ID3D11Buffer> m_IndexBuffer;

public:
	void Create(const std::vector<unsigned int>& indices) {

		// デバイス取得
		ID3D11Device* device = nullptr;

		device = RenderManager::GetInstance().GetGraphicsDevice()->GetDevice();

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
		devicecontext = RenderManager::GetInstance().GetGraphicsDevice()->GetContext();

		// インデックスバッファをセット
		devicecontext->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	}

	// インデックスバッファバッファ書き換え
	template <typename T>
	void Modify(const std::vector<T>& indices)
	{
		//頂点データ書き換え
		D3D11_MAPPED_SUBRESOURCE msr;
		HRESULT hr = RenderManager::GetInstance().GetGraphicsDevice()->GetContext()->Map(
			m_IndexBuffer.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD, 0, &msr);

		if (SUCCEEDED(hr)) {
			memcpy(msr.pData, indices.data(), indices.size() * sizeof(T));
			RenderManager::GetInstance().GetGraphicsDevice()->GetContext()->Unmap(m_IndexBuffer.Get(), 0);
		}
	}
};

