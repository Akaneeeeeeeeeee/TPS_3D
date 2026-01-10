#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "renderer.h"
#include "system/BoneCombMatrix.h"
#include "system/CMaterial.h"
#include "system/CTexture.h"
#include "system/CShader.h"

class IShader;

enum class RenderPhase
{
    OpaqueGBuffer,        // 不透明：GBuffer
    TransparentForward,   // 半透明：フォワード
    Overlay2D             // UI/フェード
};

struct DrawItem
{
    UINT indexNum = 0;          // インデックス数
    UINT indexBase = 0;
    UINT vertexBase = 0;

    CMaterial* material = nullptr;
    CTexture* diffuse = nullptr;   // 無いなら nullptr
    BoneCombMatrix* bones = nullptr; // スキン無しなら nullptr
};


/// <summary>
/// 描画に必要な情報をまとめた構造体
/// </summary>
struct RenderInfo
{
	ID3D11Buffer* vertexBuffer = nullptr;				// 頂点バッファ
	ID3D11Buffer* indexBuffer = nullptr;				// インデックスバッファ
	UINT          stride = 0;							// 頂点のサイズ
	DXGI_FORMAT   indexFormat = DXGI_FORMAT_R32_UINT;	// インデックスのフォーマット
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // プリミティブトポロジー

    // 共通
    Matrix4x4 world = Matrix4x4::Identity; // ←ポインタじゃなく値で保持（寿命事故回避）
    
    /*IShader* vs = nullptr;
    IShader* ps = nullptr;*/

	CShader* shader = nullptr; // シェーダー（VS/PS一体型）

    // このオブジェクトのサブセット列（component側が持つvectorへの参照）
    const std::vector<DrawItem>* items = nullptr;

    // パス
    RenderPhase phase = RenderPhase::OpaqueGBuffer;
};