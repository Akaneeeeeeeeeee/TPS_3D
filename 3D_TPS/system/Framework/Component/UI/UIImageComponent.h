#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/Component/Renderer/SpriteRenderer/ISpriteSource.h"
#include "CSprite.h"
#include "transform.h"

class UIManager;

class UIImageComponent final : public IComponent, public ISpriteSource
{
public:
    // UI用Transform（RectTransform相当）を「コンポーネント内に内包」
    struct UITransform
    {
        // 画面左上(0,0)～右下(1,1)の割合
        Vector2 anchor = { 0.5f, 0.5f };

        // anchor位置からの差分（px）。右+ / 下+
        Vector2 anchoredPosPx = { 0.0f, 0.0f };

        // 表示サイズ（px）
        Vector2 sizePx = { 100.0f, 100.0f };

        // 自分の基準点（0..1）(0,0)=左上, (0.5,0.5)=中心
        Vector2 pivot = { 0.5f, 0.5f };

        float rotZRad = 0.0f;

        // 描画順
        int layer = 0;
        int order = 0;

        bool visible = true;
    };

    // AddComponent("Image", texPath) を想定
    explicit UIImageComponent(const std::string& texPath)
        : m_TexPath(texPath) {
    }

    // 使い勝手用：生成後にTransformをいじれる
    UITransform& Rect() { return m_Rect; }
    const UITransform& Rect() const { return m_Rect; }

    // RenderPacket用のワールド行列生成（2D座標）
    Matrix4x4 BuildWorld2D(int screenW, int screenH) const;
    // ISpriteSource
    bool IsVisible() const override
    {
        return m_Inited && GetIsValid() && !IsDestroyRequested() && m_Rect.visible;
    }

    const CSprite* GetSprite() const override { return &m_Sprite; }

    Matrix4x4 GetWorld(int screenW, int screenH) const override
    {
        // centerPx を左上原点の座標としてそのまま使う
        Vector2 centerPx = CalcCenterPx(screenW, screenH);

        SRT srt;
        srt.scale = Vector3(m_Rect.sizePx.x, m_Rect.sizePx.y, 1.0f);
        srt.rot = Vector3(0.0f, 0.0f, m_Rect.rotZRad);
        srt.pos = Vector3(centerPx.x, centerPx.y, 0.0f);
        return srt.GetMatrix();
    }

    int GetLayer() const { return m_Rect.layer; }
    int GetOrder() const { return m_Rect.order; }

    // IComponent
    void Attach(EngineServices&) override {}   // UIManager登録をやめる
    void Detach() override {}
    void Init() override;
    void Update(float) override;
    void Uninit() override {}

private:
    // 画面ピクセル(左上原点) → Renderer2D座標 へ変換
    // ※ここだけ環境依存。ズレたらここを直す。
    static Vector3 ScreenPxToRenderer2D(const Vector2& centerPx, int w, int h);

    Vector2 CalcCenterPx(int screenW, int screenH) const;

private:
    UITransform m_Rect;

    CSprite m_Sprite;

    std::string m_TexPath;
    bool m_Inited = false;
    bool m_RequestReinit = false;
};
