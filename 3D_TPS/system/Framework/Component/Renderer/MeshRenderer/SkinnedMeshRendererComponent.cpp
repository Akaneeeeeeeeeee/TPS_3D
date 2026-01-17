#include "SkinnedMeshRendererComponent.h"

#include "system/Framework/AssetManager/AssetManager.h"
#include "system/CAnimationMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/GameObject/GameObject.h"
#include "BoneCombMatrix.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"

void SkinnedMeshRendererComponent::CollectRenderPackets(std::vector<RenderPacket>& out)
{
    if (!GetIsValid() || !m_pOwner) return;
    if (m_MeshKey.empty() || m_ShaderKey.empty()) return;

    // bones
    BoneCombMatrix* bones = nullptr;
    if (m_Anim) bones = m_Anim->GetBones();
    else        bones = m_Bones;
    if (!bones) return;

    auto& assets = AssetManager::GetInstance();

    auto* mesh = assets.GetMesh<CAnimationMesh>(m_MeshKey);
    if (!mesh) return;

    const CStaticMeshRenderer& r = mesh->GetRenderer();

    auto* shader = assets.GetShader<CShader>(m_ShaderKey);
    if (!shader) return;

    BuildDrawItems(r);               // m_DrawItems を埋める（メンバ保持）
    if (m_DrawItems.empty()) return;

    MeshDraw md{};
    md.vb = r.GetVB();
    md.ib = r.GetIB();
    md.stride = sizeof(VERTEX_SKINNED_GPU); // ★VBがこれで作られていることが前提
    md.indexFormat = r.GetIndexFormat();
    md.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    md.world = m_pOwner->GetWorldMatrix();
    md.shader = shader;
    md.items = std::span<const DrawItem>(m_DrawItems.data(), m_DrawItems.size());

    RenderPacket p{};
    p.phase = m_IsTransparent ? RenderPhase::TransparentForward : RenderPhase::OpaqueGBuffer;
    p.type = DrawType::Mesh;
    p.blend = m_IsTransparent ? BlendMode::Alpha : BlendMode::None;
    p.depthTest = true;
    p.depthWrite = !m_IsTransparent; // 透明は基本 false
    p.cull = true;

    // sortKeyは必要なら（透明は距離ソート等に後で拡張）
    p.sortKey = 0;

    p.payload = md;
    out.push_back(p);
}


void SkinnedMeshRendererComponent::BuildDrawItems(const CStaticMeshRenderer& r)
{
    // DrawItemを作る（bones を入れる）
    m_DrawItems.clear();

    // ボーンは Animator から取る
    BoneCombMatrix* bones = nullptr;
    if (m_Anim) bones = m_Anim->GetBones();
    else bones = m_Bones; // 互換のため残してOK

    const auto& subsets = r.GetSubsets();
    m_DrawItems.reserve(subsets.size());

    for (const auto& s : subsets)
    {
        DrawItem di{};
        di.indexNum = s.IndexNum;
        di.indexBase = s.IndexBase;
        di.vertexBase = s.VertexBase;

        di.material = r.GetMaterial(s.MaterialIdx);
        di.diffuse = r.GetDiffuseTextureOrNull(s.MaterialIdx);

        // 全サブセットで同じボーンCB（b5）
        di.bones = bones;

        m_DrawItems.push_back(di);
    }
}
