#pragma once
#include "Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "Framework/Component/StateIcon/EnemyHeadIconComponent.h"
#include "Framework/Graphics/RenderInfo.h"

class BillboardSpriteRenderer final : public IRenderer
{
public:
    DECLARE_COMPONENT_TYPE(BillboardSpriteRenderer, IRenderer)

    void SetSource(EnemyHeadIconComponent* src) { m_Src = src; }
    void SetDepthTest(bool e) { m_DepthTest = e; }   // falseêÑèß
    void SetLayer(int layer) { m_Layer = layer; }    // OverlayWorldì‡ÇÃèá
    void SetOrder(int order) { m_Order = order; }

    void Init() override {}
    void Update(const float) override {}
    void Uninit() override {}

    void CollectRenderPackets(std::vector<RenderPacket>& out) override;

private:
    EnemyHeadIconComponent* m_Src = nullptr; // îÒèäóL
    bool m_DepthTest = false;
    int m_Layer = 0;
    int m_Order = 0;
};
