#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "CSprite.h"

class UIManager;

class UIImageComponent final : public IComponent
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

public:
    // AddComponent("Image", texPath) を想定
    explicit UIImageComponent(const std::string& texPath)
        : m_TexPath(texPath) {
    }

    // 使い勝手用：生成後にTransformをいじれる
    UITransform& Rect() { return m_Rect; }
    const UITransform& Rect() const { return m_Rect; }

    // 任意：テクスチャ差し替え
    void SetTexture(const std::string& texPath)
    {
        m_TexPath = texPath;
        m_RequestReinit = true;
    }

    // UIManager が使う
    bool ShouldDrawUI() const;
    void DrawUI(int screenW, int screenH) const;
    int  GetLayer() const { return m_Rect.layer; }
    int  GetOrder() const { return m_Rect.order; }

    // IComponent
    void Init() override;
    void Update(const float /*deltatime*/) override;
    void Uninit() override {}

    void Attach(EngineServices& context) override;
    void Detach() override;

private:
    // 画面ピクセル(左上原点) → Renderer2D座標 へ変換
    // ※ここだけ環境依存。ズレたらここを直す。
    static Vector3 ScreenPxToRenderer2D(const Vector2& centerPx, int w, int h);

    Vector2 CalcCenterPx(int screenW, int screenH) const;

private:
    UIManager* m_pUI = nullptr;

    UITransform m_Rect;

    CSprite m_Sprite;

    std::string m_TexPath;
    bool m_Inited = false;
    bool m_RequestReinit = false;
};
