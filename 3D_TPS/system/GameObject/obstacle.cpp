#include	"obstacle.h"	
#include    "system/CDirectInput.h"
#include	"system/meshmanager.h"

#include	"Framework/Component/Physic/BoxCollider.h"
#include	"Framework/Component/Physic/Rigidbody.h"
#include "system/DebugUI.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"

void obstacle::Awake()
{
	auto& am = AssetManager::GetInstance();
	m_mesh = am.GetMesh<CStaticMesh>("obstaclebox");
	m_shader = am.GetShader<CShader>("unlightshader");
	m_meshrenderer = am.GetMeshRenderer<CStaticMeshRenderer>("obstaclebox");
	//m_mesh = MeshManager::getMesh<CStaticMesh>("obstaclebox");
	//m_shader = MeshManager::getShader<CShader>("unlightshader");
	//m_meshrenderer = MeshManager::getRenderer<CStaticMeshRenderer>("obstaclebox");

    // 描画コンポーネント
    m_RenderComp = AddComponent<StaticMeshRendererComponent>("GoalRenderer");
    m_RenderComp->SetMeshRendererKey("obstaclebox");     // AssetManagerのMeshRendererキー
    m_RenderComp->SetShaderKey("unlightshader");      // AssetManagerのShaderキー
    m_RenderComp->SetTransparent(false);


	auto boxcollider = AddComponent<BoxCollider>("fallingboxcollider");
    boxcollider->SetHalfSize(Vector3(GetScale().x, GetScale().y, GetScale().z));
    boxcollider->SetOffset(Vector3(0.0f, GetScale().y, 0.0f));

    auto rb = AddComponent<Rigidbody>("Rigidbody", 1.0f);
	rb->SetBodyType(Rigidbody::Type::Static);
	rb->SetObjectLayer(Layers::NON_MOVING);

	//DebugUI::RedistDebugFunction([this]() { DebugImGui(); });
}

void obstacle::Update(const float dt) {

	GameObject::Update(dt);
}

void obstacle::Draw(void) const {


	//Matrix4x4 mtx = m_Transform.GetWorldMatrix();

	//Renderer::SetWorldMatrix(&mtx);

	//m_shader->SetGPU();
	//m_meshrenderer->Draw();

}

void obstacle::Uninit(void)
{
	GameObject::Uninit();
}


static bool ComputeMeshBounds(
    const std::vector<VERTEX_SKINNED_GPU>& vertices,
    //const std::vector<VERTEX_3D>& vertices,
    Vector3& outMin,
    Vector3& outMax
)
{
    if (vertices.empty())
        return false;

    outMin = Vector3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
    outMax = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (auto& v : vertices)
    {
        outMin.x = std::min(outMin.x, v.Position.x);
        outMin.y = std::min(outMin.y, v.Position.y);
        outMin.z = std::min(outMin.z, v.Position.z);

        outMax.x = std::max(outMax.x, v.Position.x);
        outMax.y = std::max(outMax.y, v.Position.y);
        outMax.z = std::max(outMax.z, v.Position.z);
    }
    return true;
}

void obstacle::DebugImGui()
{
    ImGui::Begin("Obstacle Debug");

    ImGui::Text("Name: %s", m_Name.c_str());

    // Transform
    Vector3 pos = GetPosition();
    Vector3 scl = GetScale();
    Quaternion rot = GetRotation();

    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    ImGui::Text("Scale   : (%.2f, %.2f, %.2f)", scl.x, scl.y, scl.z);

    // Mesh bounds
    if (m_mesh)
    {
        Vector3 min, max;
        if (ComputeMeshBounds(m_mesh->GetVertices(), min, max))
        {
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

    // Collider shape info
    if (auto col = GetComponent<BoxCollider>())
    {
        ImGui::Separator();
        ImGui::Text("BoxCollider Offset:");
        ImGui::Text("  (%.2f, %.2f, %.2f)",
            col->GetOffset().x, col->GetOffset().y, col->GetOffset().z);

        // ★ BoxShape の half-extent を表示
        if (auto shape = col->GetShape())
        {
            const JPH::BoxShape* box = static_cast<const JPH::BoxShape*>(shape.GetPtr());
            JPH::Vec3 half = box->GetHalfExtent();

            ImGui::Separator();
            ImGui::Text("BoxCollider Shape HalfExtent:");
            ImGui::Text("  (%.2f, %.2f, %.2f)", half.GetX(), half.GetY(), half.GetZ());

            ImGui::Text("BoxCollider Full Size:");
            ImGui::Text("  (%.2f, %.2f, %.2f)",
                half.GetX() * 2.0f,
                half.GetY() * 2.0f,
                half.GetZ() * 2.0f);
        }
    }

    ImGui::End();
}

