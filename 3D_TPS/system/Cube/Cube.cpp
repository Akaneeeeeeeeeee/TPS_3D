#include "Cube.h"
#include "system/BoxDrawer.h"
#include "system/Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "system/renderer.h"
#include "Framework/Component/Physic/BoxCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"

Cube::Cube(ComponentFactory* factory, const uint64_t id,
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: GameObject(factory, id, name, tag, transform), m_Material("mymaterial")
{
}

Cube::~Cube()
{
}

void Cube::Awake(void)
{
	// ボックスメッシュの初期化(スケール準拠)
	Vector3 scale = Vector3(100.0f, 100.0f, 100.0f);
	this->SetScale(scale);
	m_BoxMesh.Init(scale.x, scale.y, scale.z, Color(1.0f, 0.0f, 0.0f, 1.0f));
	// メッシュレンダラーの初期化
	m_MeshRenderer.Init(m_BoxMesh);
	

}

void Cube::Update(const float deltatime)
{
	GameObject::Update(deltatime);
}

void Cube::Draw(void) const
{
	// ワールド行列計算
	Matrix4x4 world = this->GetWorldMatrix();
	Renderer::SetWorldMatrix(&world);

	// ベースカラーを各シェーダーの定数バッファに書き込む
	MATERIAL mtrl;
	// マテリアル生成
	mtrl.Ambient = Color(0, 0, 0, 0);
	mtrl.Diffuse = Color(1, 0, 0, 0);
	mtrl.Emission = Color(0, 0, 0, 0);
	mtrl.Specular = Color(0, 0, 0, 0);
	mtrl.Shiness = 0;
	mtrl.TextureEnable = FALSE;
	m_Material.WriteCBuffer(3, &mtrl);	// スロット3に書き込む（テスト）
	//
	//// unlittextureps/vsを使用
	//m_Material.Bind();	// ここでおかしくなる→
	//Renderer::GetDeviceContext()->DrawIndexed(m_BoxMesh.GetIndices().size(), 0, 0);
	m_MeshRenderer.Draw();
	//m_Material.Unbind();
	BoxDrawerDraw(world, Color(1, 0, 0, 1));	// これをどうにかしてIShaderの方にしたい
}

void Cube::Uninit(void)
{
	GameObject::Uninit();
}