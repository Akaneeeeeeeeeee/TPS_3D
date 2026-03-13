#include	<iostream>

#include	"LineDrawer.h"
#include	"CommonTypes.h"
#include	"CMeshRenderer.h"
#include	"CMaterial.h"
#include	"CLineMesh.h"
#include    "CShader.h"

static CLineMesh g_mesh;
static CMeshRenderer g_renderer;
static CMaterial g_material;

static CShader g_shader;
static ComPtr<ID3D11Buffer> g_linewidthbuffer;

// 線の太さ設定用定数バッファ
struct LINEWIDTHCBUFFER
{
    float widthPx;
    float invW;
    float invH;
    float pad;
};

void LineDrawerInit()
{
	g_mesh.Init(Vector3(0, 0, 0), Vector3(0, 0, 1), 1);
	g_renderer.Init(g_mesh);

	MATERIAL mtrl;
	// マテリアル生成
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;

	g_material.Create(mtrl);

	// シェーダーの初期化
	g_shader.Create(
		"shader/unlitTextureVS.hlsl",			// 頂点シェーダー
		"shader/unlitTexturePS.hlsl",			// ピクセルシェーダー
		"shader/GeometryShader.hlsl"			// ジオメトリシェーダ
	);

	// 定数バッファを作成する（線の太さを渡すため）
	bool sts = CreateConstantBuffer(
		Renderer::GetDevice(),					// デバイスオブジェクト
		sizeof(LINEWIDTHCBUFFER),				// コンスタントバッファサイズ
		g_linewidthbuffer.GetAddressOf()		// コンスタントバッファ
	);

	assert(sts);
}

void LineDrawerUninit()
{
    //g_linewidthbuffer.Reset();

    //g_shader.Uninit();
    //g_material.Uninit();
    //g_renderer.Uninit();
    //g_mesh.Uninit();
}

// ビューポートのサイズを取得
static void GetViewportSize(float& outW, float& outH)
{
    outW = 1.0f;
    outH = 1.0f;

    D3D11_VIEWPORT vp{};
    UINT num = 1;
    Renderer::GetDeviceContext()->RSGetViewports(&num, &vp);
    if (num > 0)
    {
        outW = (vp.Width > 1.0f) ? vp.Width : 1.0f;
        outH = (vp.Height > 1.0f) ? vp.Height : 1.0f;
    }
}

void SetLineWidth(float widthPx)
{
    float w, h;
    GetViewportSize(w, h);

    LINEWIDTHCBUFFER cb{};
    cb.widthPx = widthPx;
    cb.invW = 1.0f / w;
    cb.invH = 1.0f / h;

    // 定数バッファへ書き込み
    auto* ctx = Renderer::GetDeviceContext();
    ctx->UpdateSubresource(g_linewidthbuffer.Get(), 0, nullptr, &cb, 0, 0);

    // ジオメトリシェーダーにセット
    // b6はSpotLightBufferと衝突するので避ける
    ctx->GSSetConstantBuffers(10, 1, g_linewidthbuffer.GetAddressOf());
}

void LineDrawerDraw(
	float length,
	Vector3 start,
	Vector3 direction, 
	Color col)
{
	g_mesh.Clear();
	g_mesh.Init(start, direction, length);
	Matrix4x4 mtx = Matrix4x4::Identity;

	// 頂点バッファを更新
	g_renderer.Modify(g_mesh.GetVertices());

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_shader.SetGPU();
	g_material.SetGPU();
	g_renderer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

// ------------------------
// インスタンシング用
// ------------------------
static CLineMesh        g_instMesh;
static CMeshRenderer    g_instRenderer;
static CMaterial        g_instMaterial;
static CShader          g_instShader;
static ComPtr<ID3D11Buffer> g_cbInstance;

static const UINT LINE_MAX_INSTANCE = 256;

// HLSL の LineInstance と対応させる定数バッファ
struct LineInstanceGPU
{
    Vector3 start;
    float   _pad0;
    Vector3 end;
    float   _pad1;
    Color   color;   // 16byte
};

struct CBLineInstance
{
    LineInstanceGPU inst[LINE_MAX_INSTANCE];
};

void LineInstancedDrawerInit()
{
    ID3D11Device* dev = Renderer::GetDevice();

    // ベースとなる線メッシュ: (0,0,0) → (0,0,1) の 1 本
    g_instMesh.Init(Vector3(0, 0, 0), Vector3(0, 0, 1), 1.0f);
    g_instRenderer.Init(g_instMesh);

    // マテリアルは一旦白。色はインスタンス側で持つ
    MATERIAL mtrl{};
    mtrl.Ambient = Color(0, 0, 0, 0);
    mtrl.Diffuse = Color(1, 1, 1, 1);
    mtrl.Emission = Color(0, 0, 0, 0);
    mtrl.Specular = Color(0, 0, 0, 0);
    mtrl.Shiness = 0;
    mtrl.TextureEnable = FALSE;
    g_instMaterial.Create(mtrl);

    // VS: instancedLineVS.hlsl / PS: unlitTexturePS.hlsl / GS: GeometryShader.hlsl
    g_instShader.Create(
        "shader/instancedLineVS.hlsl",
        "shader/unlitTexturePS.hlsl",
        "shader/GeometryShader.hlsl");

    // インスタンス用 cbuffer (b8)
    bool sts = CreateConstantBuffer(
        dev,
        sizeof(CBLineInstance),
        g_cbInstance.GetAddressOf());
    assert(sts);
}

void LineInstancedDrawerUninit()
{
    g_cbInstance.Reset();

    //g_instShader.Uninit();
    //g_instMaterial.Uninit();
    //g_instRenderer.Uninit();
    //g_instMesh.Uninit();
}

void LineInstancedDrawerDraw(
    const std::vector<LineInstanceParam>& lines)
{
    if (lines.empty()) return;

    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();

    // マテリアル更新（色はインスタンス側で持つが、他の情報を GPU に送るため）
    g_instMaterial.Update();

    size_t total = lines.size();
    size_t offset = 0;

    while (offset < total)
    {
        size_t batchCount = std::min<size_t>(LINE_MAX_INSTANCE, total - offset);

        CBLineInstance cb{};
        for (size_t i = 0; i < batchCount; ++i)
        {
            const auto& src = lines[offset + i];
            auto& dst = cb.inst[i];

            dst.start = src.start;
            dst.end = src.end;
            dst.color = src.color;
        }

        ctx->UpdateSubresource(
            g_cbInstance.Get(),
            0,
            nullptr,
            &cb,
            0,
            0);

        // b8 にセット
        ctx->VSSetConstantBuffers(8, 1, g_cbInstance.GetAddressOf());

        g_instShader.SetGPU();
        g_instMaterial.SetGPU();

        // CLineMesh は 1 本ぶんの 2 頂点を持っている前提
        g_instRenderer.DrawInstanced(D3D11_PRIMITIVE_TOPOLOGY_LINELIST, static_cast<UINT>(batchCount));

        offset += batchCount;
    }
}