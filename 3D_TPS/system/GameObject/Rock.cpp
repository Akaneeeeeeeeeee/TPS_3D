#include "Rock.h"
#include "system/meshmanager.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "system/CMesh.h"
#include "system/CStaticMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "Framework/Component/Physic/SphereCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include <iostream>
#include "Framework/ObjectManager/ObjectManager.h"
#include "Sound/WorldSoundEvent.h"
#include "Framework/SoundManager/SoundManager.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "Framework/Component/Sound/SoundEmitterComponent.h"

Rock::Rock(ComponentFactory* factory, const uint64_t id,
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: GameObject(factory, id, name, tag, transform)
{
}

Rock::~Rock()
{
}

static float ComputeRadiusFromMesh(const CStaticMesh& mesh)
{
	const auto& verts = mesh.GetVertices();
	if (verts.empty()) return 0.0f;

	Vector3 mn = verts[0].Position;
	Vector3 mx = verts[0].Position;

	for (const auto& v : verts)
	{
		mn.x = std::min(mn.x, v.Position.x);
		mn.y = std::min(mn.y, v.Position.y);
		mn.z = std::min(mn.z, v.Position.z);

		mx.x = std::max(mx.x, v.Position.x);
		mx.y = std::max(mx.y, v.Position.y);
		mx.z = std::max(mx.z, v.Position.z);
	}

	Vector3 center = (mn + mx) * 0.5f;

	float r2 = 0.0f;
	for (const auto& v : verts)
	{
		Vector3 d = v.Position - center;
		r2 = std::max(r2, d.x * d.x + d.y * d.y + d.z * d.z);
	}

	return std::sqrt(r2); // ローカル空間の半径
}

void Rock::SetInitialVelocity(const Vector3& v)
{
	m_PendingVel = v;

	// もし既に Rigidbody が用意できてるなら即反映してOK
	if (m_RB)
	{
		m_RB->SetLinearVelocity(v);
		m_PendingVel.reset();
	}
}

void Rock::SetInitialAngularVelocity(const Vector3& av)
{
	m_PendingAngVel = av;
	// もし既に Rigidbody が用意できてるなら即反映してOK
	if (m_RB)
	{
		m_RB->SetAngularVelocity(av);
		m_PendingAngVel.reset();
	}
}

void Rock::Start()
{
	// Awake でRigidbodyが作れたので、保留速度を流す
	if (m_PendingVel.has_value())
	{
		m_RB->SetInitialVelocity(*m_PendingVel);
		m_PendingVel.reset();
	}
	if (m_PendingAngVel.has_value())
	{
		m_RB->SetInitialAngularVelocity(*m_PendingAngVel);
		m_PendingAngVel.reset();
	}
}

void Rock::Awake(void)
{
	m_Mesh = AssetManager::GetInstance().GetMesh<CStaticMesh>("Rock");
	m_MeshRenderer = AssetManager::GetInstance().GetMeshRenderer<CStaticMeshRenderer>("Rock");
	m_Shader = AssetManager::GetInstance().GetShader<CShader>("unlightshader");

	m_Transform.SetScale(Vector3(0.05f, 0.05f, 0.05f)); // とりあえず。見た目に合わせて調整

	// -------------------------
	// 物理：SphereCollider(Shape) → Rigidbody(Body)
	// -------------------------
	m_Sphere = AddComponent<SphereCollider>("RockSphere");
	m_Sphere->SetRadius(20.0f); // とりあえず。見た目に合わせて調整
	//float r = ComputeRadiusFromMesh(*m_Mesh);
	//m_Sphere->SetRadius(r);

	m_RB = AddComponent<Rigidbody>("RockRB");
	m_RB->SetBodyType(Rigidbody::Dynamic);
	m_RB->SetObjectLayer(Layers::MOVING); // Terrain/Character との設計に合わせる
	m_RB->SetMaxLinearVelocity(5000.0f, /*applyNow=*/false);

	// 描画コンポーネント
	m_pRenderComp = AddComponent<StaticMeshRendererComponent>("RockRenderer");
	m_pRenderComp->SetMeshRendererKey("Rock");
	m_pRenderComp->SetShaderKey("unlightshader");
	m_pRenderComp->SetTransparent(false);

	// 音コンポーネント
	m_pSoundEmitter = AddComponent<SoundEmitterComponent>("RockSoundEmitter");
}

void Rock::Update(const float deltatime)
{
	// デバッグ用に出力
	/*if (m_RB)
	{
		std::cout << "[Rock] Update name=" << GetName()
			<< " RB Vel=(" << m_RB->GetLinearVelocity().x << ", "
			<< m_RB->GetLinearVelocity().y << ", "
			<< m_RB->GetLinearVelocity().z << ")\n";
	}*/

	if (m_DespawnTimer >= 0.0f)
	{
		m_DespawnTimer -= deltatime;
		if (m_DespawnTimer <= 0.0f)
		{
			Destroy();
			return;
		}
	}
}

void Rock::Draw(void) const
{
}

void Rock::Uninit(void)
{
	m_Sphere = nullptr;
	m_RB = nullptr;

	m_Mesh = nullptr;
	m_MeshRenderer = nullptr;
	m_Shader = nullptr;
}

void Rock::OnCollisionEnter(GameObject& other)
{
	// すでに1回鳴らしてたら何もしない
	if (m_HitOnce) return;

	// 速度が小さい“置いただけ”接触は無視したいなら閾値
	float speed = 0.0f;
	if (m_RB)
	{
		Vector3 v = m_RB->GetLinearVelocity();
		speed = v.Length();
	}
	if (speed < 50.0f) // 調整
		return;

	m_HitOnce = true;

	// ---- 音イベント発生 ----
	// 大きさ：速度でそれっぽく（0.1〜1.0に丸め）
	float loud = std::clamp(speed / 1200.0f, 0.1f, 1.0f);
	loud = 1.0f; // とりあえず最大音量で

	if (m_pSoundEmitter)
	{
		WorldSoundEvent ev{};
		ev.Type = SoundType::StoneImpact;
		ev.Position = GetPosition();
		ev.Loudness = loud;
		ev.Radius = 600.0f * loud;		// 調整
		ev.Volume = loud;
		ev.PlayLabel = SE_STONE; // 明示したいなら
		ev.Emitter = SoundEmitterKind::PlayerItem; // 投げた石なので

		m_pSoundEmitter->EmitSound(ev);
		//auto L = ev.Position;
		//printf("[Rock] SoundPosition=(%.2f,%.2f,%.2f)!!!!!!!!!!!!!!!\n", L.x, L.y, L.z);
	}
	// ---- 少し待って消滅 ----
	m_DespawnTimer = 0.35f; // 調整（秒）
}