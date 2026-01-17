#include "UISpriteRenderer.h"
#include "Framework/Window/Window.h"

void UISpriteRenderer::CollectRenderPackets(std::vector<RenderPacket>& out)
{
    if (!GetIsValid() || !m_Src) return;
    if (!m_Src->IsVisible()) return;

    const CSprite* sp = m_Src->GetSprite();
    if (!sp) return;

    const int w = Window::GetInstance().GetWidth();
    const int h = Window::GetInstance().GetHeight();

    SpriteDraw sd{};
    sd.sprite = sp;
    sd.world = m_Src->GetWorld(w, h);
    sd.depthTest = false;

    // ‚Ç‚Á‚¿‚ğ—Dæ‚·‚é‚©‚Í“ˆê‚·‚é‚Æ—Ç‚¢B‚±‚±‚Å‚Í renderer‘¤İ’è‚ğ—DæB
    sd.layer = m_Layer;
    sd.order = m_Order;

    RenderPacket p{};
    p.phase = m_Phase;
    p.type = (m_Phase == RenderPhase::OverlayWorld)
        ? DrawType::SpriteBillboard
        : DrawType::Sprite2D;
    p.blend = BlendMode::Alpha;
    p.depthTest = false;
    p.depthWrite = false;
    p.cull = false;
    p.sortKey = (uint64_t(uint32_t(sd.layer)) << 32) | uint32_t(sd.order);
    p.payload = sd;

    out.push_back(p);
}
