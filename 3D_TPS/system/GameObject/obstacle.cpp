#include	"obstacle.h"	
#include    "system/CDirectInput.h"
#include	"system/meshmanager.h"

#include	"scene/TestScene.h"

#include	"Framework/Component/Physic/BoxCollider.h"
#include	"Framework/Component/Physic/Rigidbody.h"

void obstacle::Init()
{
	m_mesh = MeshManager::getMesh<CStaticMesh>("obstaclebox");
	m_shader = MeshManager::getShader<CShader>("unlightshader");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("obstaclebox");

	/*AddComponent<BoxCollider>("BoxCollider", DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f))->Init();
	auto rb = AddComponent<Rigidbody>("Rigidbody", 1.0f);
	rb->SetBodyType(Rigidbody::Type::Static);
	rb->Init();*/
}

void obstacle::Update(const float dt) {

	GameObject::Update(dt);
}

void obstacle::Draw(void) const {


	Matrix4x4 mtx = m_Transform.GetWorldMatrix();

	Renderer::SetWorldMatrix(&mtx);

	m_shader->SetGPU();
	m_meshrenderer->Draw();

}

void obstacle::Uninit(void)
{
	RemoveComponent(GetComponent("Rigidbody"));
	RemoveComponent(GetComponent("BoxCollider"));
}
