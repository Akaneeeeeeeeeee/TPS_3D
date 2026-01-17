#include "StaticMeshRenderer.h"

#include "system/Framework/AssetManager/AssetManager.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/GameObject/GameObject.h"


void StaticMeshRendererComponent::CollectRenderPackets(std::vector<RenderPacket>& out)
{
    if (!GetIsValid() || !m_pOwner) return;

    auto& assets = AssetManager::GetInstance();
    auto* r = assets.GetMeshRenderer<CStaticMeshRenderer>(m_MeshRendererKey);
    auto* s = assets.GetShader<CShader>(m_ShaderKey);
    if (!r || !s) return;

    BuildDrawItems(*r);
    if (m_DrawItems.empty()) return;

    MeshDraw md{};
    md.vb = r->GetVB();
    md.ib = r->GetIB();
    md.stride = sizeof(VERTEX_SKINNED_GPU);
    md.indexFormat = r->GetIndexFormat();
    md.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    md.world = m_pOwner->GetWorldMatrix();
    md.shader = s;
    md.items = std::span<const DrawItem>(m_DrawItems.data(), m_DrawItems.size());

    RenderPacket p{};
    p.phase = m_IsTransparent ? RenderPhase::TransparentForward : RenderPhase::OpaqueGBuffer;
    p.type = DrawType::Mesh;
    p.blend = m_IsTransparent ? BlendMode::Alpha : BlendMode::None;
    p.depthTest = true;
    p.depthWrite = !m_IsTransparent;
    p.cull = true;
    p.sortKey = 0; // Ç¢Ç‹ÇÕïsóvÇ»ÇÁ0Ç≈OK
    p.payload = md;

    out.push_back(std::move(p));
}


void StaticMeshRendererComponent::BuildDrawItems(const CStaticMeshRenderer& renderer)
{
    m_DrawItems.clear();

    const auto& subsets = renderer.GetSubsets();
    m_DrawItems.reserve(subsets.size());

    for (const auto& s : subsets)
    {
        DrawItem di{};
        di.indexNum = s.IndexNum;
        di.indexBase = s.IndexBase;
        di.vertexBase = s.VertexBase;

        di.material = renderer.GetMaterial(s.MaterialIdx);
        di.diffuse = renderer.GetDiffuseTextureOrNull(s.MaterialIdx);
        di.bones = nullptr; // ê√ìIÇ»ÇÃÇ≈ñ≥Çµ

        m_DrawItems.push_back(di);
    }
}
