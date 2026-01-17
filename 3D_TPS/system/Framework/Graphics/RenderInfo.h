#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <span>
#include <variant>
#include "commontypes.h"

class BoneCombMatrix;
class CMaterial;
class CTexture;
class CShader;
class CSprite;

class IShader;

enum class BlendMode : uint8_t
{
    None,
    Alpha,
    Add,
    Sub
};

enum class RenderPhase
{
    OpaqueGBuffer,        // 不透明：GBuffer
    TransparentForward,   // 半透明：フォワード
    OverlayWorld,         // 3D空間の上に載せる（頭上アイコン/線/波紋など）
    Overlay2D             // UI/フェード
};

enum class DrawType
{
    Mesh,
    SpriteBillboard,
    Sprite2D
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

struct MeshDraw
{
    ID3D11Buffer* vb = nullptr;                         // 頂点バッファ
    ID3D11Buffer* ib = nullptr;                         // インデックスバッファ
    UINT stride = 0;                                    // 頂点のサイズ
    DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT;     // インデックスのフォーマット
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;  // プリミティブトポロジー

    Matrix4x4 world = Matrix4x4::Identity;
    CShader* shader = nullptr;
    std::span<const DrawItem> items;
};

struct SpriteDraw
{
    // 「CSprite::DrawRaw(world)」だけを使う前提にして、Sprite内部で状態変更させない
    const CSprite* sprite = nullptr;
    Matrix4x4 world = Matrix4x4::Identity;

    // 2D用のソート（UI用）
    int layer = 0;
    int order = 0;

    // 3D用の深度ON/OFF（頭上アイコンはOFFが多い）
    bool depthTest = false;
};

/// <summary>
/// 描画に必要な情報をまとめた構造体
/// </summary>
struct RenderPacket
{
    RenderPhase phase = RenderPhase::OpaqueGBuffer;
    DrawType type = DrawType::Mesh;

    // パイプライン状態（RenderManagerが設定）
    BlendMode blend = BlendMode::None;
    bool depthTest = true;
    bool depthWrite = true;
    bool cull = true;

    // ソート（必要なら）
    uint64_t sortKey = 0;

	// 描画内容(どちらか一方だけ有効)
    std::variant<MeshDraw, SpriteDraw> payload;
};