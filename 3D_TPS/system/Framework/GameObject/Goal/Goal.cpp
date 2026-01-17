#include "Goal.h"
#include "system/meshmanager.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "Framework/GameObject/Player/Player.h"

void Goal::Awake(void)
{
	auto& am = AssetManager::GetInstance();
	m_mesh = am.GetMesh<CStaticMesh>("goalmesh");

	// 描画コンポーネント
	m_RenderComp = AddComponent<StaticMeshRendererComponent>("GoalRenderer");
	m_RenderComp->SetMeshRendererKey("goalmesh");
	m_RenderComp->SetShaderKey("unlightshader");
	m_RenderComp->SetTransparent(false);

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
}

void Goal::Update(const float delta)
{
}

void Goal::Draw(void) const
{
}

void Goal::Uninit(void)
{
}

void Goal::OnCollisionCharacterEnter(GameObject& other)
{
	// プレイヤーがゴールに触れたらクリア
	if (other.IsObjA<Player>())
	{
		m_Reached = true;
	}
}