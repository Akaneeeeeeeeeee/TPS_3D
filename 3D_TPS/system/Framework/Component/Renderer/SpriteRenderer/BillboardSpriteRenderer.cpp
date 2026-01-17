#include "BillboardSpriteRenderer.h"


void BillboardSpriteRenderer::CollectRenderPackets(std::vector<RenderPacket>& out)
{
    if (!GetIsValid() || !m_Src) return;
    if (!m_Src->IsVisible()) return;

    const CSprite* sp = m_Src->GetCurrentSprite();
    if (!sp) return;

    SpriteDraw sd{};
    sd.sprite = sp;
    sd.world = m_Src->GetWorld();
    sd.depthTest = m_DepthTest;
    sd.layer = m_Layer;
    sd.order = m_Order;

    RenderPacket p{};
    p.phase = RenderPhase::OverlayWorld;
    p.type = DrawType::SpriteBillboard;
    p.blend = BlendMode::Alpha;
    p.depthTest = m_DepthTest;
    p.depthWrite = false;  // Overlay‚ÍŠî–{‘‚©‚È‚¢
    p.cull = false;
    p.sortKey = (uint64_t(uint32_t(m_Layer)) << 32) | uint32_t(m_Order);
    p.payload = sd;

    out.push_back(p);
}