#include "EnemyHeadIconComponent.h"
#include "system/renderer.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/Component/AI/EnemyAIComponent.h"
#include "system/Framework/Component/Camera/CameraComponent.h"
#include "system/CSprite.h"

EnemyHeadIconComponent::EnemyHeadIconComponent()
{
}

EnemyHeadIconComponent::~EnemyHeadIconComponent()
{
    Uninit();
}

void EnemyHeadIconComponent::Setup(CameraComponent* camera,
    int width, int height,
    const std::string& questionTex,
    const std::string& alertTex)
{
    m_pCamera = camera;

    // 2枚を初期化
    m_QuestionSprite.Init(width, height, questionTex);
    m_AlertSprite.Init(width, height, alertTex);

    m_pQuestion = &m_QuestionSprite;
    m_pAlert = &m_AlertSprite;
}

void EnemyHeadIconComponent::Init(void)
{
    // Owner から AI を取る（Enemy クラスに依存しない）
    if (m_pOwner)
    {
        m_pAI = m_pOwner->GetComponent<EnemyAIComponent>("EnemyAI");
    }
}

void EnemyHeadIconComponent::Uninit(void)
{
    m_QuestionSprite.Dispose();
    m_AlertSprite.Dispose();

    m_pQuestion = nullptr;
    m_pAlert = nullptr;

    m_pCamera = nullptr;
    m_pAI = nullptr;
}

EnemyHeadIconComponent::IconKind EnemyHeadIconComponent::PickIcon() const
{
    if (!m_pAI) return IconKind::None;

    const auto st = m_pAI->GetState();

    // Idle/Patrolは表示なし
    if (st == EnemyAIComponent::State::Idle || st == EnemyAIComponent::State::Patrol)
        return IconKind::None;

    // 怪しい/調査中 → 疑問
    if (st == EnemyAIComponent::State::Caution || st == EnemyAIComponent::State::Investigate)
        return IconKind::Question;

    return IconKind::None;
}

const CSprite* EnemyHeadIconComponent::GetSprite(IconKind k) const
{
    switch (k)
    {
    case IconKind::Question: return m_pQuestion;
    case IconKind::Alert:    return m_pAlert;
    default:                 return nullptr;
    }
}

void EnemyHeadIconComponent::Update(const float /*dt*/)
{
    // 必須情報がなければ何もしない
    if (!m_pCamera || !m_pAI) return;

    // 頭上位置
    Vector3 pos = m_pOwner->GetTransform().GetPosition() + m_Offset;

    // ビルボード行列作成
    Vector3 look = m_pCamera->GetPosition() - pos;
    look.Normalize();

    // 上下反転解決用
    Matrix4x4 scale = Matrix4x4::CreateScale(Vector3(1, -1, 1));
    Matrix4x4 bb = Matrix4x4::CreateBillboard(pos, m_pCamera->GetPosition(), Vector3::Up, &look);

    m_World = scale * bb;
}

void EnemyHeadIconComponent::Draw(void) const
{
}
