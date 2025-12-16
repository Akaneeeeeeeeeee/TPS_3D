#include "Rock.h"
#include "system/Framework/Component/Renderer/MeshRenderer/MeshRenderer.h"
#include "system/meshmanager.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "system/CMesh.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"

Rock::Rock(ComponentFactory* factory, const uint64_t id,
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: GameObject(factory, id, name, tag, transform)
{
}

Rock::~Rock()
{
	Uninit();
}

void Rock::Awake(void)
{
	m_Mesh = AssetManager::GetInstance().GetStaticMesh("Rock");
	m_MeshRenderer = MeshManager::getRenderer<CStaticMeshRenderer>("obstaclerock");
	m_Shader = MeshManager::getShader<CShader>("unlightshader");

	auto meshcolider = AddComponent<StaticMeshCollider>("RockMeshcollider");
	meshcolider->SetMesh(m_Mesh->GetVertices(), m_Mesh->GetIndices());
}

void Rock::Update(const float deltatime)
{
	GameObject::Update(deltatime);
}

void Rock::Draw(void) const
{
	Matrix4x4 mtx = m_Transform.GetWorldMatrix();

	Renderer::SetWorldMatrix(&mtx);

	m_Shader->SetGPU();
	m_MeshRenderer->Draw();
}

void Rock::Uninit(void)
{
	GameObject::Uninit();
}