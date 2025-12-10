#include "Goal.h"
#include "system/meshmanager.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"

void Goal::Init(void)
{
	m_mesh = MeshManager::getMesh<CStaticMesh>("goalmesh");
	m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("goalmesh");
	m_shader = MeshManager::getShader<CShader>("unlightshader");

	// コライダー
	{
		auto collider = AddComponent<StaticMeshCollider>("GoalMeshCollider");
		collider->SetMesh(*m_mesh);
	}

	// Rigidbody
	{
		auto rb = AddComponent<Rigidbody>("Rigidbody", 1.0f);
		rb->SetBodyType(Rigidbody::Static);
		rb->SetObjectLayer(Layers::NON_MOVING);
	}

	GameObject::Init();
}

void Goal::Update(const float delta)
{
	GameObject::Update(delta);
}

void Goal::Draw(void) const
{
	Matrix4x4 mtx = m_Transform.GetWorldMatrix();
	Renderer::SetWorldMatrix(&mtx);
	m_shader->SetGPU();
	m_meshrenderer->Draw();
}

void Goal::Uninit(void)
{
	GameObject::Uninit();
}