#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

class IShader;

struct CBufferBinding
{
    UINT slot;                 // b# slot
    std::string name;          // debug or matching name
    ID3D11Buffer* buffer;      // GPU buffer (ID3D11Buffer*)

    const void* cpuData;       // ← CPU 側データのアドレス
    UINT dataSize;             // ← バイト数
};

struct SRVBinding
{
    UINT slot;                     // t# スロット番号
	std::string name;          // リソース名（オプション）
    ID3D11ShaderResourceView* view; // SRV
};


/// <summary>
/// 1つのサブセット描画に必要な情報をまとめた構造体
/// </summary>
struct RenderInfo {
	// todo: RenderPhase phase / Material* material / BoneCB 等を追加

    // --- 頂点・インデックス ---
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT;
    UINT stride = 0;          // 頂点サイズ
    UINT indexCount = 0;

    UINT startIndex = 0;   // DrawIndexed に渡す開始インデックス
    UINT baseVertex = 0;   // DrawIndexed に渡す頂点ベース

    // --- シェーダー ---
    class IShader* vs = nullptr;
    class IShader* ps = nullptr;

    // --- 描画ごとの定数バッファ ---
    std::vector<CBufferBinding> cBuffers; // ワールド行列、マテリアル、ライト、ビュー/プロジェクションなど

    // --- 描画ごとのテクスチャ ---
    std::vector<SRVBinding> srvs;         // Albedo, Normal, Roughness, etc.

    // --- Transformやスケールなどの情報が必要な場合 ---
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

    // --- オプションの情報 ---
    bool visible = true;   // 描画対象かどうか
    int renderOrder = 0;   // 描画順ソート用


    // ============================
    // スキニング用追加メンバ
    // ============================
    // ※スタティックメッシュは使わないので nullptr のまま
    ID3D11Buffer* boneMatrixCB = nullptr;
    int boneCount = 0;
};