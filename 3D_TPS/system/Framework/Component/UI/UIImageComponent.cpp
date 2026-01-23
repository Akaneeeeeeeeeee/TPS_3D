#include "UIImageComponent.h"

void UIImageComponent::Init()
{
    // 1x1で作って scale で伸ばす
    m_Sprite.Init(1, 1, m_TexPath);
    m_Inited = true;
    m_RequestReinit = false;
}

void UIImageComponent::Update(const float /*dt*/)
{
    if (m_RequestReinit)
    {
        m_Sprite.Init(1, 1, m_TexPath);
        m_RequestReinit = false;
        m_Inited = true;
    }
}

Vector2 UIImageComponent::CalcCenterPx(int screenW, int screenH) const
{
    const Vector2 anchorPx = {
        screenW * m_Rect.anchor.x,
        screenH * m_Rect.anchor.y
    };

    // anchoredPos は pivot位置
    const Vector2 pivotPx = {
        anchorPx.x + m_Rect.anchoredPosPx.x,
        anchorPx.y + m_Rect.anchoredPosPx.y
    };

    // spriteは中心基準なので中心へ
    const Vector2 centerPx = {
        pivotPx.x + (0.5f - m_Rect.pivot.x) * m_Rect.sizePx.x,
        pivotPx.y + (0.5f - m_Rect.pivot.y) * m_Rect.sizePx.y
    };

    return centerPx;
}

Matrix4x4 UIImageComponent::BuildWorld2D(int screenW, int screenH) const
{
    const Vector2 centerPx = CalcCenterPx(screenW, screenH);

    // SetWorldViewProjection2D() が OffCenterLH なので
    // 座標は「左上原点(0,0)、右+、下+」でそのまま使う
    SRT srt;
    srt.scale = Vector3(m_Rect.sizePx.x, m_Rect.sizePx.y, 1.0f);
    srt.rot = Vector3(0.0f, 0.0f, m_Rect.rotZRad);
    srt.pos = Vector3(centerPx.x, centerPx.y, 0.0f);

    return srt.GetMatrix();
}
