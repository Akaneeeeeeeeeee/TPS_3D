#pragma once
#include <array>
#include <string>
#include "CSprite.h"
#include "system/commontypes.h"
#include "system/Framework/Component/IComponent/IComponent.h"
#include "system/transform.h"

class CSprite;

struct SpriteDrawCmd
{
    const CSprite* sprite = nullptr;
    Matrix4x4      world = Matrix4x4::Identity;
    int            layer = 0;      // 小さいほど手前(または先に描く)など、ルールは統一
    bool           visible = true;
};

enum class UIAnchor
{
    TopLeft, Top, TopRight,
    Left, Center, Right,
    BottomLeft, Bottom, BottomRight
};

class SpriteRendererComponent  : public IComponent 
{
public:
    void Init(int width, int height, const std::string& tex,
        std::array<Vector2, 4> uv = { Vector2(0,0), Vector2(1,0), Vector2(0,1), Vector2(1,1) })
    {
        m_Sprite.Init(width, height, tex, uv);
        m_SizePx = Vector2((float)width, (float)height);
    }

    // ---- UI設定 ----
    void SetAnchor(UIAnchor a) { m_Anchor = a; }
    void SetPositionPx(const Vector2& p) { m_PosPx = p; }        // アンカー基準からのオフセット(px)
    void SetRotationRad(float r) { m_RotRad = r; }
    void SetScale(const Vector2& s) { m_Scale = s; }
    void SetLayer(int layer) { m_Layer = layer; }
    void SetVisible(bool v) { m_Visible = v; }

    // ---- 提出（描画システムに「描け」と渡す）----
    void Submit(std::vector<SpriteDrawCmd>& outCmds, int screenW, int screenH) const
    {
        if (!m_Visible) return;

        SpriteDrawCmd cmd;
        cmd.sprite = &m_Sprite;
        cmd.layer = m_Layer;
        cmd.visible = true;
        cmd.world = CalcWorldMatrix(screenW, screenH);

        outCmds.push_back(cmd);
    }

private:
    Matrix4x4 CalcWorldMatrix(int screenW, int screenH) const
    {
        // アンカー位置（画面座標）
        Vector2 anchorPos = CalcAnchorPos(screenW, screenH);

        // CSpriteは中心原点なので、pos = (アンカー位置 + オフセット) が中心点になる
        Vector3 pos(anchorPos.x + m_PosPx.x, anchorPos.y + m_PosPx.y, 0.0f);

        // 2DなのでZ回転だけ使う想定（あなたのSRTがXYZ回転ならZに入れる）
        Vector3 rot(0.0f, 0.0f, m_RotRad);

        // scale は「倍率」。サイズ自体はCSpriteの頂点で持っているので 1 が基本
        Vector3 scale(m_Scale.x, m_Scale.y, 1.0f);

        SRT srt;
        srt.pos = pos;
        srt.rot = rot;
        srt.scale = scale;

        return srt.GetMatrix();
    }

    Vector2 CalcAnchorPos(int screenW, int screenH) const
    {
        const float w = (float)screenW;
        const float h = (float)screenH;

        switch (m_Anchor)
        {
        case UIAnchor::TopLeft:     return Vector2(0.0f, 0.0f);
        case UIAnchor::Top:         return Vector2(w * 0.5f, 0.0f);
        case UIAnchor::TopRight:    return Vector2(w, 0.0f);
        case UIAnchor::Left:        return Vector2(0.0f, h * 0.5f);
        case UIAnchor::Center:      return Vector2(w * 0.5f, h * 0.5f);
        case UIAnchor::Right:       return Vector2(w, h * 0.5f);
        case UIAnchor::BottomLeft:  return Vector2(0.0f, h);
        case UIAnchor::Bottom:      return Vector2(w * 0.5f, h);
        case UIAnchor::BottomRight: return Vector2(w, h);
        }
        return Vector2(w * 0.5f, h * 0.5f);
    }

private:
    Vector2  m_SizePx = Vector2(0, 0);
    CSprite  m_Sprite{};

    UIAnchor m_Anchor = UIAnchor::Center;
    Vector2  m_PosPx = Vector2(0, 0);   // アンカーからのオフセット(px)
    Vector2  m_Scale = Vector2(1, 1);
    float    m_RotRad = 0.0f;
    int      m_Layer = 0;
    bool     m_Visible = true;
};
