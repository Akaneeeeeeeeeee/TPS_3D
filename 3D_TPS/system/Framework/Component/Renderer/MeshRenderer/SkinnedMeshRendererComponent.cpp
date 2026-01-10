#include "SkinnedMeshRendererComponent.h"

#include "system/Framework/AssetManager/AssetManager.h"
#include "system/CAnimationMesh.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/GameObject/GameObject.h"
#include "BoneCombMatrix.h"
#include "Framework/Component/Animator/SkinnedAnimatorComponent.h"

bool SkinnedMeshRendererComponent::GetRenderInfo(RenderInfo& outInfo)
{
    if (!GetIsValid() || !m_pOwner) return false;
    if (m_MeshKey.empty() || m_ShaderKey.empty()) return false;

    auto& assets = AssetManager::GetInstance();

    // メッシュ(Akai)を取る
    auto* mesh = assets.GetMesh<CAnimationMesh>(m_MeshKey);
    if (!mesh) return false;

    // メッシュ内蔵のレンダラを使う
    const CStaticMeshRenderer& r = mesh->GetRenderer();

    // シェーダ（スキン用）
    auto* shader = assets.GetShader<CShader>(m_ShaderKey);
    if (!shader) return false;

    BuildDrawItems(r);
    if (m_DrawItems.empty()) return false;

    // VB/IB は CStaticMeshRenderer 側の getter が必要
    outInfo.vertexBuffer = r.GetVB();
    outInfo.indexBuffer = r.GetIB();
    outInfo.stride = sizeof(VERTEX_SKINNED_GPU);
    //outInfo.stride = sizeof(VERTEX_3D);
    outInfo.indexFormat = r.GetIndexFormat();
    outInfo.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    outInfo.world = m_pOwner->GetWorldMatrix();

    outInfo.shader = shader;
    outInfo.items = &m_DrawItems;

    outInfo.phase = m_IsTransparent ? RenderPhase::TransparentForward : RenderPhase::OpaqueGBuffer;
    return true;
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
