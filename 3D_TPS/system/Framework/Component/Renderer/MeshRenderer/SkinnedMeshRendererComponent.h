#pragma once
#include "Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "Framework/AssetManager/AssetManager.h"
#include "Framework/Graphics/RenderInfo.h"
#include "system/CStaticMeshRenderer.h"

// 前方宣言
class GameObject;
class CShader;
class SkinnedAnimationComponent;
struct BoneCombMatrix;

// スキンメッシュ描画コンポーネント
// - メッシュ/サブセット/マテリアル/テクスチャは AssetManager の CStaticMeshRenderer を参照
// - ボーン行列は “このインスタンス” の BoneCombMatrix を使う（b5）
// Skinned = メッシュ(Akai)は共有、ボーン行列は個体ごと（b5）
class SkinnedMeshRendererComponent final : public IRenderer
{
public:
	DECLARE_COMPONENT_TYPE(SkinnedMeshRendererComponent, IRenderer)
    SkinnedMeshRendererComponent() = default;
    ~SkinnedMeshRendererComponent() override = default;

    void Init() override {}
    void Update(const float) override {}
    void Uninit() override {}

    // AssetManagerに登録した CAnimationMesh のキー
    void SetMeshKey(const std::string& key) { m_MeshKey = key; }

    // AssetManagerに登録した CShader のキー
    void SetShaderKey(const std::string& key) { m_ShaderKey = key; }

    // Animator等が更新するボーン定数バッファ（b5）
    void SetBones(BoneCombMatrix* bones) { m_Bones = bones; }

    void SetTransparent(bool transparent) { m_IsTransparent = transparent; }

    void SetAnimator(SkinnedAnimationComponent* anim) { m_Anim = anim; }

    //bool GetRenderInfo(RenderInfo& outInfo) override;
	void CollectRenderPackets(std::vector<RenderPacket>& out) override;

private:
    void BuildDrawItems(const class CStaticMeshRenderer& r);

private:
    std::string m_MeshKey;
    std::string m_ShaderKey;

    SkinnedAnimationComponent* m_Anim = nullptr; // 非所有
    BoneCombMatrix* m_Bones = nullptr;
    bool m_IsTransparent = false;

    // RenderInfo.items が参照するのでメンバで保持
    std::vector<DrawItem> m_DrawItems;
};