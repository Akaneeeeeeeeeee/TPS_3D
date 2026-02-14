#include	"SphereDrawer.h"
#include	"CylinderDrawer.h"
#include    "CCylinderMesh.h"
#include	"CSphereMesh.h"
#include	"CMeshRenderer.h"
#include	"CapsuleDrawer.h"
#include	"CMaterial.h"
#include	"CShader.h"

static CCylinderMesh g_meshcylinder;
static CSphereMesh g_meshsphere;

static CMeshRenderer g_renderercylinder;
static CMeshRenderer g_renderersphere;

static CMaterial g_material;
static CShader g_shader;

void CapsuleDrawerInit(void) 
{
	g_meshsphere.Init(1, Color(1, 1, 1, 1), 100, 100);
	g_renderersphere.Init(g_meshsphere);

	g_meshcylinder.Init(50,		// 分割数
		1,						// 半径
		1,						// 高さ
		Color(1, 1, 1, 1));

	g_renderercylinder.Init(g_meshcylinder);

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


void CapsuleDrawerDraw(float radius, float height, Color col, float posx, float posy, float posz)
{
    // 半分の高さを計算（上下の球を除いた円柱部分の高さ）
    float halfCylHeight = std::max(0.0f, height * 0.5f - radius);

    // シェーダ設定
    g_shader.SetGPU();
    g_material.SetDiffuse(col);
    g_material.Update();
    g_material.SetGPU();

    // 位置行列のベース
    Matrix4x4 base;
    base.Identity;
    base._41 = posx;
    base._42 = posy;
    base._43 = posz;

    // 中央の円柱部分
    {
        Matrix4x4 mtx = Matrix4x4::CreateScale(radius * 2.0f, halfCylHeight * 2.0f, radius * 2.0f);
        mtx._41 = posx;
        mtx._42 = posy;
        mtx._43 = posz;

        Renderer::SetWorldMatrix(&mtx);
        g_renderercylinder.Draw();
    }

    // 上の球
    {
        Matrix4x4 mtx = Matrix4x4::CreateScale(radius * 2.0f, radius * 2.0f, radius * 2.0f);
        mtx._41 = posx;
        mtx._42 = posy + halfCylHeight + radius;
        mtx._43 = posz;

        Renderer::SetWorldMatrix(&mtx);
        g_renderersphere.Draw();
    }

    // 下の球
    {
        Matrix4x4 mtx = Matrix4x4::CreateScale(radius * 2.0f, radius * 2.0f, radius * 2.0f);
        mtx._41 = posx;
        mtx._42 = posy - halfCylHeight - radius;
        mtx._43 = posz;

        Renderer::SetWorldMatrix(&mtx);
        g_renderersphere.Draw();
    }
}
