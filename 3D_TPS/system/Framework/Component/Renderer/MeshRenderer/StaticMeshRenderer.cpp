#include "StaticMeshRenderer.h"

#include "system/Framework/AssetManager/AssetManager.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/GameObject/GameObject.h"

bool StaticMeshRendererComponent::GetRenderInfo(RenderInfo& outInfo)
{
    if (!GetIsValid() || !m_pOwner) return false;
    if (m_MeshRendererKey.empty() || m_ShaderKey.empty()) return false;

    // Asset から参照を取得
    auto& assets = AssetManager::GetInstance();
    auto* r = assets.GetMeshRenderer<CStaticMeshRenderer>(m_MeshRendererKey);
    auto* s = assets.GetShader<CShader>(m_ShaderKey);

    if (!r || !s) return false;

    // サブセット→DrawItem 生成（メンバに保持）
    BuildDrawItems(*r);
    if (m_DrawItems.empty()) return false;

    // VB/IB（CStaticMeshRenderer に getter を追加してある前提）
    outInfo.vertexBuffer = r->GetVB();
    outInfo.indexBuffer = r->GetIB();
    outInfo.stride = sizeof(VERTEX_SKINNED_GPU); // 頂点構造体に合わせる
    //outInfo.stride = sizeof(VERTEX_3D); // 頂点構造体に合わせる
    outInfo.indexFormat = r->GetIndexFormat();
    outInfo.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // ワールド行列（あなたのGameObjectに合わせて取得）
    outInfo.world = m_pOwner->GetWorldMatrix(); // ←無ければ Transform から作る関数に差し替え

    outInfo.shader = s;

    // items はメンバ vector のアドレス（寿命保証）
    outInfo.items = &m_DrawItems;

    // パス設定
    outInfo.phase = m_IsTransparent ? RenderPhase::TransparentForward : RenderPhase::OpaqueGBuffer;

    return true;
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
        di.bones = nullptr; // 静的なので無し

        m_DrawItems.push_back(di);
    }
}
