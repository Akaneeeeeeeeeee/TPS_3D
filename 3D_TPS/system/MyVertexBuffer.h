#pragma once
#include	<vector>
#include	<wrl/client.h>
#include	"dx11helper.h"
#include	"renderer.h"
#include	"NonCopyable.h"


using Microsoft::WRL::ComPtr;

template <typename T>
class MyVertexBuffer final : public NonCopyable{

	ComPtr<ID3D11Buffer> m_VertexBuffer;
	UINT m_Stride = sizeof(T);
	UINT m_VertexCount = 0;

public:
	void Create(const std::vector<T>& vertices) {

		// デバイス取得
		ID3D11Device* device = nullptr;
		device = Renderer::GetDevice();
		assert(device);
		// 頂点数取得
		m_VertexCount = static_cast<UINT>(vertices.size());

		// 頂点バッファ作成
		bool sts = CreateVertexBufferWrite(
			device,
			m_Stride,						// １頂点当たりバイト数
			m_VertexCount,					// 頂点数
			(void*)vertices.data(),			// 頂点データ格納メモリ先頭アドレス
			&m_VertexBuffer);				// 頂点バッファ

		assert(sts == true);
	}

	// GPUにセット
	void SetGPU() {

		// デバイスコンテキスト取得
		ID3D11DeviceContext* devicecontext = nullptr;
		devicecontext = Renderer::GetDeviceContext();

		// 頂点バッファをセットする
		unsigned int stride = sizeof(T);
		unsigned  offset = 0;
		devicecontext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);

	}

	// 頂点バッファを書き換える
	void Modify(const std::vector<T>& vertices)
	{
		//頂点データ書き換え
		D3D11_MAPPED_SUBRESOURCE msr;
		HRESULT hr = Renderer::GetDeviceContext()->Map(
			m_VertexBuffer.Get(), 
			0,
			D3D11_MAP_WRITE_DISCARD, 0, &msr);

		if (SUCCEEDED(hr)) {
			memcpy(msr.pData, vertices.data(), vertices.size() * sizeof(T));
			Renderer::GetDeviceContext()->Unmap(m_VertexBuffer.Get(), 0);
		}
	}

	ID3D11Buffer* GetBuffer(void)
	{
		// バッファのポインタを返す
		return m_VertexBuffer.Get();
	}

	UINT GetStride(void)
	{
		// 頂点データのサイズを返す
		return m_Stride;
	}

	UINT GetVertexCount(void)
	{
		return m_VertexCount;
	}
};
