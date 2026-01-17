#pragma once
#include "Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "Framework/Component/Renderer/SpriteRenderer/ISpriteSource.h"
#include "Framework/Graphics/RenderInfo.h"

class UISpriteRenderer final : public IRenderer
{
public:
    DECLARE_COMPONENT_TYPE(UISpriteRenderer, IRenderer)

    void SetSource(ISpriteSource* src) { m_Src = src; }
    void SetLayer(int layer) { m_Layer = layer; }
    void SetDepthTest(bool e) { m_DepthTest = e; }
    void SetOrder(int order) { m_Order = order; }
    void SetPhase(RenderPhase ph) { m_Phase = ph; }

    void Init() override {}
    void Update(const float) override {}
    void Uninit() override {}

    void CollectRenderPackets(std::vector<RenderPacket>& out) override;

private:
    ISpriteSource* m_Src = nullptr; // îÒèäóL
    bool m_DepthTest = false;
    int m_Layer = 0;
    int m_Order = 0;
    RenderPhase m_Phase = RenderPhase::Overlay2D;
};
