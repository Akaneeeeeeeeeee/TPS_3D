#include	<iostream>
#include   "CommonTypes.h"

#include	"CMeshRenderer.h"
#include	"CMaterial.h"
#include	"CSphereMesh.h"
#include    "CShader.h"
#include	"transform.h"


static CSphereMesh g_mesh;
static CMeshRenderer g_renderer;
static CMaterial g_material;
static CShader g_shader;

static CSphereMesh           g_instMesh;
static CMeshRenderer         g_instRenderer;
static CMaterial             g_instMaterial;
static CShader               g_instShader;
static ComPtr<ID3D11Buffer>  g_cbCamera;
static ComPtr<ID3D11Buffer>  g_cbInstance;

static const UINT MAX_INSTANCE = 256;

// カメラ情報
struct CBInstSphereCamera
{
	Matrix4x4 view;
	Matrix4x4 proj;
};

struct CBInstSphereInstance
{
	Matrix4x4 world[MAX_INSTANCE];
};

void SphereDrawerInit() 
{
	g_mesh.Init(1, Color(1, 1, 1, 1), 50, 50);
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
		"shader/unlitTextureVS.hlsl",			// 頂点シェーダー
		"shader/unlitTexturePS.hlsl");			// ピクセルシェーダー

}

void SphereDrawerDraw(float radius,Color col,float ex, float ey, float ez)
{
	Matrix4x4 mtx = Matrix4x4::CreateScale(radius);

	mtx._41 = ex;
	mtx._42 = ey;
	mtx._43 = ez;

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_shader.SetGPU();

	g_material.SetGPU();
	g_renderer.Draw();
}

void SphereDrawerDraw(SRT srt ,Color col)
{
	Matrix4x4 mtx = srt.GetMatrix();

	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_shader.SetGPU();

	g_material.SetGPU();
	g_renderer.Draw();
}

void SphereDrawerDraw(Matrix4x4 mtx, Color col)
{
	Renderer::SetWorldMatrix(&mtx);
	g_material.SetDiffuse(col);
	g_material.Update();

	g_shader.SetGPU();

	g_material.SetGPU();
	g_renderer.Draw();
}

// インスタンシング用初期化
void SphereInstancedDrawerInit()
{
	ID3D11Device* dev = Renderer::GetDevice();

	// メッシュ（普通の球）
	g_instMesh.Init(1.0f, Color(1, 1, 1, 1), 32, 32);
	g_instRenderer.Init(g_instMesh);

	// マテリアル（単色）
	MATERIAL mtrl;
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 1, 1, 1);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;
	g_instMaterial.Create(mtrl);

	// シェーダ（インスタンス用）
	g_instShader.Create(
		"shader/instancedSphereVS.hlsl",
		"shader/instancedSpherePS.hlsl"
	);

	// カメラ用 cbuffer
	bool sts = CreateConstantBuffer(
		dev,
		sizeof(CBInstSphereCamera),
		g_cbCamera.GetAddressOf());
	assert(sts);

	// インスタンス用 cbuffer
	sts = CreateConstantBuffer(
		dev,
		sizeof(CBInstSphereInstance),
		g_cbInstance.GetAddressOf());
	assert(sts);
}

void SphereInstancedDrawerDraw(
	const Matrix4x4& view,
	const Matrix4x4& proj,
	const std::vector<Vector3>& centers,
	float radius,
	Color col)
{
	if (centers.empty()) return;

	ID3D11DeviceContext* ctx = Renderer::GetDeviceContext();

	// ===== 1) カメラ行列を cbuffer に詰める =====
	CBInstSphereCamera cam{};

	cam.view = view;
	cam.proj = proj;

	ctx->UpdateSubresource(g_cbCamera.Get(), 0, nullptr, &cam, 0, 0);

	// b0 にカメラ行列
	ctx->VSSetConstantBuffers(7, 1, g_cbCamera.GetAddressOf());

	// マテリアル（色だけ使う）
	g_instMaterial.SetDiffuse(col);
	g_instMaterial.Update(); // b3 にセットされる

	// ===== 2) インスタンスを MAX_INSTANCE ごとに描く =====

	size_t total = centers.size();
	size_t offset = 0;

	while (offset < total)
	{
		size_t batchCount = std::min<size_t>(MAX_INSTANCE, total - offset);

		CBInstSphereInstance inst{};
		for (size_t i = 0; i < batchCount; ++i)
		{
			const Vector3& c = centers[offset + i];

			Matrix4x4 scale = Matrix4x4::CreateScale(radius, radius, radius);
			Matrix4x4 trans = Matrix4x4::CreateTranslation(c.x, c.y, c.z);
			Matrix4x4 world = scale * trans;

			inst.world[i] = world; // 転置
		}

		ctx->UpdateSubresource(g_cbInstance.Get(), 0, nullptr, &inst, 0, 0);

		// b1 にインスタンス行列配列
		ctx->VSSetConstantBuffers(8, 1, g_cbInstance.GetAddressOf());

		// シェーダ・マテリアルをセット
		g_instShader.SetGPU();
		g_instMaterial.SetGPU();

		// メッシュをバインドしてインスタンシング描画
		g_instRenderer.DrawInstanced(static_cast<UINT>(batchCount));

		offset += batchCount;
	}
}
