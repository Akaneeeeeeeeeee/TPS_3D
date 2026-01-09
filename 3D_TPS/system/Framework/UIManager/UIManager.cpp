#include "UIManager.h"
#include "Framework/Component/UI/UIImageComponent.h"
#include "system/renderer.h"

void UIManager::Register(UIImageComponent* img)
{
    if (!img) return;
    for (auto* p : m_Images) if (p == img) return; // 二重登録防止
    m_Images.push_back(img);
}

void UIManager::Unregister(UIImageComponent* img)
{
    if (!img) return;
    auto it = std::find(m_Images.begin(), m_Images.end(), img);
    if (it != m_Images.end()) m_Images.erase(it);
}

void UIManager::Draw(int screenW, int screenH)
{
    std::stable_sort(m_Images.begin(), m_Images.end(),
        [](const UIImageComponent* a, const UIImageComponent* b)
        {
            if (a->GetLayer() != b->GetLayer()) return a->GetLayer() < b->GetLayer();
            return a->GetOrder() < b->GetOrder();
        });

    Renderer::SetDepthEnable(false);
    Renderer::SetBlendState(BS_ALPHABLEND);
    // UI描画の共通設定はここでまとめる（理想）
    // ※CSprite側で SetWorldViewProjection2D() を呼ぶ設計のままでも動く
    Renderer::SetWorldViewProjection2D();

    for (auto* img : m_Images)
    {
        if (!img) continue;
        if (!img->ShouldDrawUI()) continue;
        img->DrawUI(screenW, screenH);
    }

    // 描き終わったら戻す（必要なら）
    Renderer::SetBlendState(BS_NONE);
    Renderer::SetDepthEnable(true);
}
