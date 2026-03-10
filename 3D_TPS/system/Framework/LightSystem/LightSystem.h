#pragma once
#include <vector>
#include <array>
#include <algorithm>
#include "CommonTypes.h"
#include "system/renderer.h"

class SpotLightComponent;
struct SpotLightGPU;
class PhysicsManager;

class LightSystem
{
public:
    static constexpr int MAX_SPOT = 64;

    void Register(SpotLightComponent* c)
    {
        if (!c) return;
        if (std::find(m_Spots.begin(), m_Spots.end(), c) != m_Spots.end()) return;
        m_Spots.push_back(c);
    }

    void Unregister(SpotLightComponent* c)
    {
        if (!c) return;
        m_Spots.erase(std::remove(m_Spots.begin(), m_Spots.end(), c), m_Spots.end());
    }

    // 毎フレーム1回：キャッシュ更新（BuildGPUをここで1回だけ）
    void UpdateCache();

    // 3D描画の直前：GPUへ
    void UploadToGPU();

    bool IsOccludedWorld(const Vector3& from, const Vector3& to) const;

    // 知覚用：指定位置の “ライトによる視認性(0..1)” を返す
    float GetLightVisibility01(const Vector3& worldPos) const;

    // 係数にしたい場合
    float GetVisibilityMul(const Vector3& worldPos) const
    {
        float t = GetLightVisibility01(worldPos); // 0..1
        return 1.0f + t * (m_MaxMul - 1.0f);
    }

    // 遮蔽判定をやりたい場合だけセット
    void SetPhysics(PhysicsManager* pm) { m_Physics = pm; }
    void SetOcclusionEnabled(bool e) { m_UseOcclusion = e; }

	float ComputeLightVisibilityForPlayer(const class Player& player) const;

    size_t GetSpotCount(void) const { return m_Cache.size(); }
    const SpotLightGPU& GetSpotGPU(int i) const { return m_Cache[i].gpu; }
private:
    struct CachedSpot
    {
        SpotLightGPU gpu{};
        SpotLightComponent* src = nullptr; // 逆参照（知覚計算用）
    };

    std::vector<SpotLightComponent*> m_Spots;
    std::vector<CachedSpot> m_Cache{};

    // 係数の最大（調整用）
    float m_MaxMul = 2.0f;

    // 遮蔽（壁で隠れる）用（必要なら）
    PhysicsManager* m_Physics = nullptr;
    bool m_UseOcclusion = false;
};
