#include "Terrain.h"

#include "system/meshmanager.h"
#include "renderer.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "system/Framework/Component/Physic/StaticMeshCollider.h"
#include "system/Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/Scene/IScene.h"
#include "Framework/GameObject/Player/Player.h"
#include "Framework/GameObject/Player/TitlePlayerActor.h"

#include "system/DebugUI.h"

void Terrain::Awake()
{
    // メッシュ／レンダラー／シェーダー取得
	auto& am = AssetManager::GetInstance();
    m_mesh = am.GetMesh<CStaticMesh>("terrainmesh");
    m_meshrenderer = am.GetMeshRenderer<CStaticMeshRenderer>("terrainmesh");
    m_shader = am.GetShader<CShader>("unlightshader"); // 実際のキー名に合わせる
    //m_mesh = MeshManager::getMesh<CStaticMesh>("terrainmesh");
    //m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("terrainmesh");
    //m_shader = MeshManager::getShader<CShader>("unlightshader"); // 実際のキー名に合わせる

    // 物理メッシュコライダを追加
    if (m_mesh)
    {
        auto col = AddComponent<StaticMeshCollider>("StaticMeshCollider");

		// メッシュデータを渡す
        const auto& verts = m_mesh->GetVertices();
        const auto& inds = m_mesh->GetIndices();

        //col->SetMesh(verts, inds);
		col->SetMesh(*m_mesh);
        m_collider = col;
    }

    // 必要ならデバッグ UI 登録
    //DebugUI::RedistDebugFunction([this]() { DebugImGui(); });
}

void Terrain::Update(const float delta)
{
    GameObject::Update(delta);
}

void Terrain::Draw() const
{
    if (!m_mesh || !m_meshrenderer || !m_shader) { return; }

    Matrix4x4 mtx = m_Transform.GetWorldMatrix();
	Matrix4x4 currentview = Renderer::GetViewMatrix();
    Matrix4x4 view;
    Matrix4x4 currentproj = Renderer::GetProjectionMatrix();
	Matrix4x4 proj;
    Renderer::SetWorldMatrix(&mtx);
    if (Player* player = m_ownerscene->GetObjectManager()->GetObjectByTag<Player>(Tag::Player))
    {
        auto comp = player->GetComponent<CameraComponent>();
        view = comp->GetViewMatrix();
		proj = comp->GetProjMatrix();
    }
    else if (TitlePlayerActor * titlePlayer = m_ownerscene->GetObjectManager()->GetObjectByTag<TitlePlayerActor>(Tag::Player))
    {
        auto comp = titlePlayer->GetComponent<CameraComponent>();
        view = comp->GetViewMatrix();
        proj = comp->GetProjMatrix();
    }

    //if(view == currentview && proj == currentproj)
    //{
    //    // 変更がないなら設定し直す必要はない
    //}
    //else
    //{
    //    // 変更があったら設定し直す
    //    Renderer::SetViewMatrix(&view);
    //    Renderer::SetProjectionMatrix(&proj);
    //}

	Renderer::SetViewMatrix(&view);
	Renderer::SetProjectionMatrix(&proj);

    m_shader->SetGPU();
    m_meshrenderer->Draw();
}

void Terrain::Uninit()
{
    GameObject::Uninit();
}

void Terrain::DebugImGui()
{
    ImGui::Begin("Terrain Debug");

    ImGui::Text("Name: %s", m_Name.c_str());

    Vector3 pos = GetPosition();
    Vector3 scl = GetScale();
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    ImGui::Text("Scale   : (%.2f, %.2f, %.2f)", scl.x, scl.y, scl.z);

    // メッシュのバウンディング情報などを出したければ obstacle と同じ要領で
    if (m_mesh)
    {
        const auto& verts = m_mesh->GetVertices();
        if (!verts.empty())
        {
            Vector3 min(+FLT_MAX, +FLT_MAX, +FLT_MAX);
            Vector3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
            for (auto& v : verts)
            {
                min.x = std::min(min.x, v.Position.x);
                min.y = std::min(min.y, v.Position.y);
                min.z = std::min(min.z, v.Position.z);

                max.x = std::max(max.x, v.Position.x);
                max.y = std::max(max.y, v.Position.y);
                max.z = std::max(max.z, v.Position.z);
            }

            Vector3 size = max - min;
            Vector3 half = size * 0.5f;

            ImGui::Separator();
            ImGui::Text("Mesh Bounds:");
            ImGui::Text("  Min : (%.2f, %.2f, %.2f)", min.x, min.y, min.z);
            ImGui::Text("  Max : (%.2f, %.2f, %.2f)", max.x, max.y, max.z);
            ImGui::Text("  Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);
            ImGui::Text("  Half: (%.2f, %.2f, %.2f)", half.x, half.y, half.z);
        }
    }

    ImGui::End();
}

bool Terrain::GetWorldXZBounds(Vector3& outMin, Vector3& outMax) const
{
    return m_collider ? m_collider->GetWorldXZBounds(outMin, outMax) : false;
}

bool Terrain::SampleHeight(float x, float z, float& outY) const
{
    return m_collider ? m_collider->SampleHeight(x, z, outY) : false;
}
