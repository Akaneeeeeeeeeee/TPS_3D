#include    "CommonTypes.h"
#include	<iostream>

#include    "BoxDrawer.h"
#include	"transform.h"
#include	"CMeshRenderer.h"
#include	"CMaterial.h"
#include	"CBoxMesh.h"
#include    "CShader.h"

static CBoxMesh g_mesh;
static CMeshRenderer g_renderer;
static CMaterial g_material;
static CShader g_shader;

void BoxDrawerInit()
{
	g_mesh.Init(
		1,					// 幅
		1,					// 高さ
		1,					// 奥行
		Color(1,1,1,1));

	g_renderer.Init(g_mesh);

	// シェーダーの初期化
	g_shader.Create(
		"shader/unlitTextureVS.hlsl",				// 頂点シェーダー
		"shader/unlitTexturePS.hlsl");			// ピクセルシェーダー

	MATERIAL mtrl;
	// マテリアル生成
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 0, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;
	g_material.Create(mtrl);
}

void BoxDrawerDraw(float width, float height, float depth, 
	Color col, float posx, float posy, float posz)
{
	Matrix4x4 mtx = Matrix4x4::CreateScale(width,height,depth);

	mtx._41 = posx;
	mtx._42 = posy;
	mtx._43 = posz;

	g_shader.SetGPU();

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_material.SetGPU();
	g_renderer.Draw();
}

void BoxDrawerDraw(SRT rts,Color col)
{
	Matrix4x4 mtx = rts.GetMatrix();

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();
	g_material.SetGPU();

	g_shader.SetGPU();

	g_renderer.Draw();
}

void BoxDrawerDraw(Matrix4x4 mtx, Color col)
{
	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();
	g_material.SetGPU();

	g_shader.SetGPU();

	g_renderer.Draw();
}


// ============ インスタンス用 ============

static CBoxMesh       g_instBoxMesh;
static CMeshRenderer  g_instBoxRenderer;
static CMaterial      g_instBoxMaterial;
static CShader        g_instBoxShader;
static ComPtr<ID3D11Buffer> g_instBoxCBCamera;
static ComPtr<ID3D11Buffer> g_instBoxCBInstance;

static const UINT BOX_MAX_INSTANCE = 256;

// カメラ用 cbuffer
struct CBInstBoxCamera
{
    Matrix4x4 view;
    Matrix4x4 proj;
};

// インスタンス用 cbuffer
struct CBInstBoxInstance
{
    Matrix4x4 world[BOX_MAX_INSTANCE];
};

void BoxInstancedDrawerInit()
{
    ID3D11Device* dev = Renderer::GetDevice();

    // メッシュは単体版と同じ形状でよい
    g_instBoxMesh.Init(1.0f, 1.0f, 1.0f, Color(1, 1, 1, 1));
    g_instBoxRenderer.Init(g_instBoxMesh);

    MATERIAL mtrl{};
    mtrl.Ambient = Color(0, 0, 0, 0);
    mtrl.Diffuse = Color(1, 1, 1, 1);
    mtrl.Emission = Color(0, 0, 0, 0);
    mtrl.Specular = Color(0, 0, 0, 0);
    mtrl.Shiness = 0;
    mtrl.TextureEnable = FALSE;
    g_instBoxMaterial.Create(mtrl);

    // ここでは既存の instancedSphere 用 VS/PS が「単に world*view*proj 掛けるだけ」
    // という前提で流用している。内容が形状依存なら専用の VS/PS を用意してもよい。
    g_instBoxShader.Create(
        "shader/instancedSphereVS.hlsl",
        "shader/instancedSpherePS.hlsl");

    // カメラ cbuffer
    bool ok = CreateConstantBuffer(
        dev,
        sizeof(CBInstBoxCamera),
        g_instBoxCBCamera.GetAddressOf());
    assert(ok);

    // インスタンス cbuffer
    ok = CreateConstantBuffer(
        dev,
        sizeof(CBInstBoxInstance),
        g_instBoxCBInstance.GetAddressOf());
    assert(ok);
}

void BoxInstancedDrawerDraw(
    const Matrix4x4& view,
    const Matrix4x4& proj,
    const std::vector<BoxInstance>& instances,
    Color col)
{
    if (instances.empty()) return;

    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();

    // 1) カメラ行列
    CBInstBoxCamera cam{};
    cam.view = view;
    cam.proj = proj;
    ctx->UpdateSubresource(g_instBoxCBCamera.Get(), 0, nullptr, &cam, 0, 0);
    ctx->VSSetConstantBuffers(7, 1, g_instBoxCBCamera.GetAddressOf()); // スロットは Sphere と合わせている

    // マテリアル（色だけ使用）
    g_instBoxMaterial.SetDiffuse(col);
    g_instBoxMaterial.Update(); // b3 等に設定される想定
    g_instBoxMaterial.SetGPU();

    size_t total = instances.size();
    size_t offset = 0;

    while (offset < total)
    {
        size_t batchCount = std::min<size_t>(BOX_MAX_INSTANCE, total - offset);

        CBInstBoxInstance inst{};
        for (size_t i = 0; i < batchCount; ++i)
        {
            inst.world[i] = instances[offset + i].world;
        }

        ctx->UpdateSubresource(
            g_instBoxCBInstance.Get(),
            0,
            nullptr,
            &inst,
            0,
            0);

        ctx->VSSetConstantBuffers(8, 1, g_instBoxCBInstance.GetAddressOf());

        g_instBoxShader.SetGPU();

        g_instBoxRenderer.DrawInstanced(static_cast<UINT>(batchCount));

        offset += batchCount;
    }
}