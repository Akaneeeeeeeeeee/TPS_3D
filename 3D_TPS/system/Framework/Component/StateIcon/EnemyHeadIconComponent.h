#pragma once
#include <string>

#include "Framework/Component/IComponent/IComponent.h"
#include "commontypes.h"
#include "system/CSprite.h"

class CameraComponent;
class EnemyAIComponent;
class CSprite;

class EnemyHeadIconComponent final : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(EnemyHeadIconComponent, IComponent)
    EnemyHeadIconComponent();
    ~EnemyHeadIconComponent() override;

    // 生成直後に呼ぶ（テクスチャはファイル名で渡す前提）
    void Setup(
        CameraComponent* camera,
        int width, int height,
        const std::string& questionTex,
        const std::string& alertTex
    );

    void SetOffset(const Vector3& offset) { m_Offset = offset; }
    void SetScale(const Vector3& scale) { m_Scale = scale; }

    void Attach(EngineServices& context) override {};
    void Detach(void) override {};

    // ---- IComponent ----
    void Init(void) override;
    void Update(const float dt) override;
    void Uninit(void) override;

    // ※ IComponent に Draw が無い場合は、下の「呼び出し口」案のどれかを使う
    void Draw(void) const;

    bool IsVisible() const { return PickIcon() != IconKind::None; }
    const CSprite* GetCurrentSprite() const { return GetSprite(PickIcon()); }
    const Matrix4x4& GetWorld() const { return m_World; }

private:
    enum class IconKind { None, Question, Alert };

    IconKind PickIcon() const;
    const CSprite* GetSprite(IconKind k) const;

private:
    CameraComponent* m_pCamera = nullptr;
    EnemyAIComponent* m_pAI = nullptr;

    // 2枚だけ
    CSprite* m_pQuestion = nullptr;
    CSprite* m_pAlert = nullptr;

    // 実体はメンバで保持（ポインタは参照用）
    CSprite  m_QuestionSprite;
    CSprite  m_AlertSprite;

    Vector3  m_Offset = Vector3(0.0f, 160.0f, 0.0f);
    Vector3  m_Scale = Vector3(1.0f, 1.0f, 1.0f);

    mutable Matrix4x4 m_World{};
};
