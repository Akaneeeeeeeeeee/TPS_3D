#include "UIImageComponent.h"
#include "Framework/UIManager/UIManager.h"
#include "Framework/GameObject/GameObject.h"
#include "transform.h" // SRT / Matrix4x4

void UIImageComponent::Attach(EngineServices& context)
{
    // EngineServicesに ui がある前提
    m_pUI = &context.ui;
    if (m_pUI) m_pUI->Register(this);
}

void UIImageComponent::Detach()
{
    if (m_pUI) m_pUI->Unregister(this);
    m_pUI = nullptr;
}

void UIImageComponent::Init()
{
    // 1x1で作って、表示サイズは scale で作る（頂点更新を避ける）
    m_Sprite.Init(1, 1, m_TexPath);

    m_Inited = true;
    m_RequestReinit = false;
}

void UIImageComponent::Update(const float /*deltatime*/)
{
    // テクスチャ差し替え要求が来たら作り直し（必要なら）
    if (m_RequestReinit)
    {
        // あなたのCSprite/Textureが再Initに対応してない場合は
        // ここを「別Spriteに差し替え」等に変更してください。
        m_Sprite.Init(1, 1, m_TexPath);
        m_RequestReinit = false;
        m_Inited = true;
    }
}

bool UIImageComponent::ShouldDrawUI() const
{
    if (!m_Inited) return false;
    if (!GetIsValid()) return false;
    if (IsDestroyRequested()) return false;
    if (!m_Rect.visible) return false;
    return true;
}

Vector2 UIImageComponent::CalcCenterPx(int screenW, int screenH) const
{
    // anchor位置（左上基準ピクセル）
    const Vector2 anchorPx = {
        screenW * m_Rect.anchor.x,
        screenH * m_Rect.anchor.y
    };

    // anchoredPosは「pivot位置」として扱う（Unity寄り）
    const Vector2 pivotPx = {
        anchorPx.x + m_Rect.anchoredPosPx.x,
        anchorPx.y + m_Rect.anchoredPosPx.y
    };

    // CSpriteは中心基準で描くので「中心ピクセル」を作る
    const Vector2 centerPx = {
        pivotPx.x + (0.5f - m_Rect.pivot.x) * m_Rect.sizePx.x,
        pivotPx.y + (0.5f - m_Rect.pivot.y) * m_Rect.sizePx.y
    };

    return centerPx;
}

void UIImageComponent::DrawUI(int screenW, int screenH) const
{
    if (!ShouldDrawUI()) return;

    const Vector2 centerPx = CalcCenterPx(screenW, screenH);
    const Vector3 pos = ScreenPxToRenderer2D(centerPx, screenW, screenH);

    SRT srt;
    srt.scale = Vector3(m_Rect.sizePx.x, m_Rect.sizePx.y, 1.0f);
    srt.rot = Vector3(0.0f, 0.0f, m_Rect.rotZRad);
    srt.pos = pos;

    Matrix4x4 world = srt.GetMatrix();

    // 推奨：UIManagerがWVPを一括設定 → DrawRawで描く
    m_Sprite.DrawRaw(world);

    // もし DrawRaw を追加しないなら、こっちに戻す
    // m_Sprite.Draw(world);
}

Vector3 UIImageComponent::ScreenPxToRenderer2D(const Vector2& centerPx, int w, int h)
{
    // 想定：SetWorldViewProjection2D が「画面中心(0,0)」「上が+Y」
    const float x = centerPx.x - w * 0.5f;
    const float y = (h * 0.5f) - centerPx.y;
    return Vector3(x, y, 0.0f);

    // もし「左上(0,0)、下が+Y」なら：
    // return Vector3(centerPx.x, centerPx.y, 0.0f);
}
