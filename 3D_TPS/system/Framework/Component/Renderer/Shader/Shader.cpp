#include "Shader.h"
#include "../../../../Game/EntryPoint/main.h"
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#include <locale.h>
#include <atltypes.h> // CRectを使うためのヘッダーファイル
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "../../../Graphics/Graphics.h"
#include "../../../DX11Helper/dx11helper.h"


/**
 * @brief 初期化
 * @return 初期化の結果
 * 
 * ここでシェーダーを作成
*/
void Shader::Init(const std::string& vsFile, const std::string& vsEntryPoint, 
	const std::string& psFile, const std::string& psEntryPoint, D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements)
{
	bool sts;

	//// インプットレイアウト作成
	//// POSITION → XMFLOAT3 だから DXGI_FORMAT_R32G32B32_FLOAT
	//// TEXCOORD → XMFLOAT2 だから DXGI_FORMAT_R32G32_FLOAT
	//// 0, 12 の部分で、それぞれのデータがバッファ内でどこにあるかを指定している
	//D3D11_INPUT_ELEMENT_DESC layout[]
	//{
	//	// 位置座標があるということを伝える
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	// 色情報があるということを伝える
	//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	// UV座標
	//	{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};


	// 各シェーダーをコンパイルして作成(バージョン名は決まってるので引数は使わない(○○_5.0とか))
	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	sts = CreateVertexShader(Graphics::GetInstance().GetDevice(), vsFile.c_str(), vsEntryPoint.c_str(),
		"vs_5_0", layout, numElements, &m_pVertexShader, &m_pVertexLayout);
	if (!sts) {
		MessageBoxA(NULL, "CreateVertexShader error", "error", MB_OK);
		return;
	}

	// ピクセルシェーダーオブジェクトを生成
	sts = CreatePixelShader(Graphics::GetInstance().GetDevice(), psFile.c_str(), psEntryPoint.c_str(), "ps_5_0", &m_pPixelShader);
	if (!sts) {
		MessageBoxA(NULL, "CreatePixelShader error", "error", MB_OK);
		return;
	}

}


void Shader::Update(void)
{
	// 描画のための情報をGPUに渡す
	auto devicecontext = Graphics::GetInstance().GetDeviceContext();
	devicecontext->IASetInputLayout(m_pVertexLayout.Get());
	devicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	devicecontext->VSSetShader(m_pVertexShader.Get(), NULL, 0);		// ここで描画に使うシェーダファイルを適用してる
	devicecontext->PSSetShader(m_pPixelShader.Get(), NULL, 0);
}


void Shader::Uninit(void)
{

}


ID3D11VertexShader* Shader::GetVertexShader(void)
{
	return m_pVertexShader.Get();
}

ID3D11PixelShader* Shader::GetPixelShader(void)
{
	return m_pPixelShader.Get();
}

