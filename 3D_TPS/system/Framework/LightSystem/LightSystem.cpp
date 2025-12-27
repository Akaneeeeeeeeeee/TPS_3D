#include "LightSystem.h"
#include "Framework/Component/Light/SpotLightComponent.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include "Framework/GameObject/Player/Player.h"
#include "system/renderer.h"

bool LightSystem::IsOccludedWorld(const Vector3& from, const Vector3& to) const
{
    if (!m_Physics) return false;            // 物理が無いなら遮蔽なし扱い
    return m_Physics->IsOccluded(from, to);  // Jolt依存は PhysicsManager に閉じ込める
}

void LightSystem::UpdateCache()
{
    m_CacheCount = 0;

    for (auto* c : m_Spots)
    {
        if (!c) continue;
        if (m_CacheCount >= MAX_SPOT) break;

        SpotLightGPU gpu{};
        if (c->BuildGPU(gpu))
        {
            m_Cache[m_CacheCount].valid = true;
            m_Cache[m_CacheCount].gpu = gpu;
            m_Cache[m_CacheCount].src = c;
            ++m_CacheCount;
        }
    }

    for (int i = m_CacheCount; i < MAX_SPOT; ++i)
    {
        m_Cache[i].valid = false;
        m_Cache[i].src = nullptr;
    }
}

void LightSystem::UploadToGPU()
{
    SpotLightGPU gpu[MAX_SPOT]{};
    int count = 0;

    for (int i = 0; i < m_CacheCount; ++i)
    {
        if (!m_Cache[i].valid) continue;
        gpu[count++] = m_Cache[i].gpu;
    }

    Renderer::SetSpotLights(gpu, count);
}

float LightSystem::GetLightVisibility01(const Vector3& worldPos) const
{
    float best = 0.0f;

    for (int i = 0; i < m_CacheCount; ++i)
    {
        if (!m_Cache[i].valid || !m_Cache[i].src) continue;

        // ① 円錐内 + 減衰（0..1）
        float t = m_Cache[i].src->ComputeInfluence01(worldPos);
        if (t <= 0.0f) continue;

        // ② 遮蔽（任意）
        if (m_UseOcclusion)
        {
            const auto& p = m_Cache[i].gpu.Position;
            Vector3 lightPos(p.x, p.y, p.z);

            // ライト位置から少し押し出す（ライトが壁に埋まってる時の即ヒット対策）
            // ※ComputeInfluence01 の内部で方向が取れるなら、そこで押し出す方が綺麗
            Vector3 from = lightPos;

            if (IsOccludedWorld(from, worldPos))
                t = 0.0f;
        }

        if (t > best) best = t;
    }

    return best;
}

float LightSystem::ComputeLightVisibilityForPlayer(const Player& player) const
{
    // ここは GetLightVisibility01 をサンプル点で最大化するだけに寄せると保守が楽
    std::vector<Vector3> samples;
    Vector3 dummy = player.GetPosition();
    player.GetVisibilitySamplePoints(dummy, samples);

    float best = 0.0f;
    for (const auto& p : samples)
    {
        float t = GetLightVisibility01(p);
        if (t > best) best = t;
        if (best >= 0.999f) return 1.0f;
    }
    return best;
}
