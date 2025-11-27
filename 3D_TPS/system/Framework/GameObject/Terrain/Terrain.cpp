#include "Terrain.h"

#include "system/meshmanager.h"
#include "renderer.h"
#include "CStaticMesh.h"
#include "CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "system/Framework/Component/Physic/StaticMeshCollider.h"

#include "system/DebugUI.h"

void Terrain::Init()
{
    // メッシュ／レンダラー／シェーダー取得
    m_mesh = MeshManager::getMesh<CStaticMesh>("terrainmesh");
    m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("terrainmesh");
    m_shader = MeshManager::getShader<CShader>("unlightshader"); // 実際のキー名に合わせる

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
    DebugUI::RedistDebugFunction([this]() { DebugImGui(); });
}

void Terrain::Update(const float delta)
{
    GameObject::Update(delta);
}

void Terrain::Draw() const
{
    if (!m_mesh || !m_meshrenderer || !m_shader)
        return;

    Matrix4x4 mtx = m_Transform.GetWorldMatrix();
    Renderer::SetWorldMatrix(&mtx);

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
