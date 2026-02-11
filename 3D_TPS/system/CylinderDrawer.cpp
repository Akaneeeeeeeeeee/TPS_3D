#include	<iostream>

#include	"CommonTypes.h"
#include	"CMeshRenderer.h"
#include	"CMaterial.h"
#include	"CSphereMesh.h"
#include	"CCylinderMesh.h"
#include	"CShader.h"
#include	"transform.h"
#include    "CylinderDrawer.h"


static CCylinderMesh g_mesh;
static CMeshRenderer g_renderer;
static CMaterial g_material;
static CShader g_shader;

void CylinderDrawerDraw(float radius, float hieght, Color col, float posx, float posy, float posz);

void CylinderDrawerInit()
{
	g_mesh.Init(50,			// 分割数
		1,					// 半径
		1,					// 高さ
		Color(1,1,1,1));

	g_renderer.Init(g_mesh);

	MATERIAL mtrl;
	// マテリアル生成
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 0, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;

	g_material.Create(mtrl);

	// シェーダーの初期化
	g_shader.Create(
		"shader/unlitTextureVS.hlsl",				// 頂点シェーダー
		"shader/unlitTexturePS.hlsl");			// ピクセルシェーダー

}

void CylinderDrawerUninit()
{
    //g_shader.Uninit();
    //g_material.Uninit();
    //g_renderer.Uninit();
    //g_mesh.Uninit();
}

void CylinderDrawerDraw(float radius, float height,Color col, float posx, float posy, float posz)
{
	Matrix4x4 mtx = Matrix4x4::CreateScale(radius,height,radius);

	mtx._41 = posx;
	mtx._42 = posy;
	mtx._43 = posz;

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_material.SetGPU();
	g_renderer.Draw();
}

void CylinderDrawerDraw(SRT rts,Color col)
{
	Matrix4x4 mtx = rts.GetMatrix();

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_material.SetGPU();
	g_renderer.Draw();
}

void CylinderDrawerDraw(Matrix4x4 mtx, Color col)
{
	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_shader.SetGPU();	

	g_material.SetGPU();
	g_renderer.Draw();
}

static CCylinderMesh          g_rainMesh;
static CMeshRenderer          g_rainRenderer;
static CMaterial              g_rainMaterial;
static CShader                g_rainShader;
static ComPtr<ID3D11Buffer>   g_cbCamera;
static ComPtr<ID3D11Buffer>   g_cbInstance;

static const UINT RAIN_MAX_INSTANCE = 256;

// カメラ行列
struct CBRainCamera
{
    Matrix4x4 view;
    Matrix4x4 proj;
};

// インスタンス行列
struct CBRainInstance
{
    Matrix4x4 world[RAIN_MAX_INSTANCE];
};

void RainInstancedDrawerInit()
{
    ID3D11Device* dev = Renderer::GetDevice();

    // 円柱メッシュ。高さ1,半径1を基準にしておいて、ワールド行列で伸ばす
    g_rainMesh.Init(
        16,                 // 分割数
        1.0f,               // 半径
        1.0f,               // 高さ
        Color(1, 1, 1, 1)); // 頂点カラー
    g_rainRenderer.Init(g_rainMesh);

    // マテリアル（色は後で SetDiffuse で変える）
    MATERIAL mtrl{};
    mtrl.Ambient = Color(0, 0, 0, 0);
    mtrl.Diffuse = Color(1, 1, 1, 1);
    mtrl.Emission = Color(0, 0, 0, 0);
    mtrl.Specular = Color(0, 0, 0, 0);
    mtrl.Shiness = 0;
    mtrl.TextureEnable = FALSE;
    g_rainMaterial.Create(mtrl);

    // シェーダ（インスタンス用。レイアウトが SphereInstanced と同じなら再利用可）
    g_rainShader.Create(
        "shader/instancedSphereVS.hlsl",   // instancedSphereVS と同じ内容でもOK
        "shader/instancedSpherePS.hlsl"
    );

    // カメラ用 cbuffer
    bool ok = CreateConstantBuffer(
        dev,
        sizeof(CBRainCamera),
        g_cbCamera.GetAddressOf());
    assert(ok);

    // インスタンス用 cbuffer
    ok = CreateConstantBuffer(
        dev,
        sizeof(CBRainInstance),
        g_cbInstance.GetAddressOf());
    assert(ok);
}

void RainInstancedDrawerUninit()
{
    g_cbInstance.Reset();
    g_cbCamera.Reset();

    //g_rainShader.Uninit();
    //g_rainMaterial.Uninit();
    //g_rainRenderer.Uninit();
    //g_rainMesh.Uninit();
}

void RainInstancedDrawerDraw(
    const Matrix4x4& view,
    const Matrix4x4& proj,
    const std::vector<RainInstance>& instances,
    float radius,
    Color col)
{
    if (instances.empty()) return;

    ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();

    // --- カメラ行列 ---
    CBRainCamera cam{};
    cam.view = view;
    cam.proj = proj;
    ctx->UpdateSubresource(g_cbCamera.Get(), 0, nullptr, &cam, 0, 0);
    ctx->VSSetConstantBuffers(7, 1, g_cbCamera.GetAddressOf());

    // マテリアル色
    g_rainMaterial.SetDiffuse(col);
    g_rainMaterial.Update();

    size_t total = instances.size();
    size_t offset = 0;

    while (offset < total)
    {
        size_t batchCount = std::min<size_t>(RAIN_MAX_INSTANCE, total - offset);

        CBRainInstance inst{};
        for (size_t i = 0; i < batchCount; ++i)
        {
            const RainInstance& r = instances[offset + i];

            // 雨1本分のワールド行列
            // ここでは Y 方向に「length」、X,Z に「radius」
            Matrix4x4 scale = Matrix4x4::CreateScale(radius, r.length, radius);
            Matrix4x4 trans = Matrix4x4::CreateTranslation(r.pos.x, r.pos.y, r.pos.z);
            Matrix4x4 world = scale * trans;

            inst.world[i] = world;
        }

        ctx->UpdateSubresource(g_cbInstance.Get(), 0, nullptr, &inst, 0, 0);
        ctx->VSSetConstantBuffers(8, 1, g_cbInstance.GetAddressOf());

        // シェーダ・マテリアル・メッシュ
        g_rainShader.SetGPU();
        g_rainMaterial.SetGPU();
        g_rainRenderer.DrawInstanced(static_cast<UINT>(batchCount));

        offset += batchCount;
    }
}