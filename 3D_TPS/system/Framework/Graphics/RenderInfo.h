#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

class IShader;

/// <summary>
/// 描画に必要な情報をまとめた構造体
/// </summary>
struct RenderInfo {
	ID3D11Buffer* vertexBuffer = nullptr;				// 頂点バッファ
	ID3D11Buffer* indexBuffer = nullptr;				// インデックスバッファ
	UINT          stride = 0;							// 頂点のサイズ
	UINT          indexCount = 0;						// インデックス数
	DXGI_FORMAT   indexFormat = DXGI_FORMAT_R32_UINT;	// インデックスのフォーマット

	const DirectX::XMMATRIX* world = nullptr;			// ワールド変換行列

	IShader* vs = nullptr;							// 頂点シェーダー
	IShader* ps = nullptr;							// ピクセルシェーダー

	// 将来: RenderPhase phase / Material* material / BoneCB 等を追加
};