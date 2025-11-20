#include "EnemyHearingComponent.h"
#include "Framework/GameObject/GameObject.h"
#include "EnemyAIComponent.h"

void EnemyHearingComponent::Update(const float dt)
{
    // 今フレーム発生した音一覧を取得
    const auto& sounds = SoundManager::Get().GetEvents();
    if (sounds.empty())
    {
        return;
    }

    Vector3 myPos = m_pOwner->GetPosition();

    float bestScore = 0.0f;
    Vector3 bestPos = myPos;
    bool found = false;

    for (const auto& s : sounds)
    {
        Vector3 toSound = s.Position - myPos;
        float distSq = toSound.LengthSquared();

        // 影響半径の外なら無視
        if (distSq > s.Radius * s.Radius)
        {
            continue;
        }

        // 距離と 音の大きさ から簡易スコアを計算
        float score = s.Loudness / (1.0f + distSq);

        if (score > bestScore)
        {
            bestScore = score;
            bestPos = s.Position;
            found = true;
        }
    }

    if (!found || bestScore < m_MinScore) { return; }

    // 敵AI本体に通知
    if (m_pEnemyAI)
    {
        m_pEnemyAI->OnHeardSound(bestPos);
    }
}

void EnemyHearingComponent::SetEnemyAI(EnemyAIComponent* ai)
{
    m_pEnemyAI = ai;
}