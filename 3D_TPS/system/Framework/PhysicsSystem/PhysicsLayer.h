#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <array>

// レイヤー定義
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;  // 静的障害物
    static constexpr JPH::ObjectLayer MOVING = 1;  // 動く物体
    static constexpr JPH::ObjectLayer CHARACTER = 2;  // プレイヤーなど
    static constexpr JPH::ObjectLayer TRIGGER = 3;  // 当たり判定専用
    static constexpr JPH::ObjectLayer TERRAIN = 4;  // 地形
    static constexpr JPH::ObjectLayer NUM_LAYERS = 5;
}

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer STATIC_BP = JPH::BroadPhaseLayer(0);
    static constexpr JPH::BroadPhaseLayer DYNAMIC_BP = JPH::BroadPhaseLayer(1);
    static constexpr uint32_t NUM = 2;
}

// -------------------------
// BroadPhaseLayerInterface
// -------------------------
class BPLayerInterface : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterface()
    {
        // デフォルトは全部 DYNAMIC_BP にしておいて、
        // 静的なものだけ STATIC_BP にマップ
        mMap.fill(BroadPhaseLayers::DYNAMIC_BP);

        mMap[Layers::NON_MOVING] = BroadPhaseLayers::STATIC_BP;
        mMap[Layers::TERRAIN] = BroadPhaseLayers::STATIC_BP;

        mMap[Layers::MOVING] = BroadPhaseLayers::DYNAMIC_BP;
        mMap[Layers::CHARACTER] = BroadPhaseLayers::DYNAMIC_BP;
        mMap[Layers::TRIGGER] = BroadPhaseLayers::DYNAMIC_BP;
    }

    uint32_t GetNumBroadPhaseLayers() const override {
        return BroadPhaseLayers::NUM;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
        return mMap[layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
        switch (layer.GetValue()) {
        case 0: return "STATIC_BP";
        case 1: return "DYNAMIC_BP";
        default: return "UNKNOWN_BP";
        }
    }
#endif

private:
    std::array<JPH::BroadPhaseLayer, Layers::NUM_LAYERS> mMap;
};

// -------------------------
// ObjectVsBroadPhaseLayerFilter
// -------------------------
class ObjectVsBPLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer obj, JPH::BroadPhaseLayer bp) const override
    {
        switch (obj) {
        case Layers::NON_MOVING:
        case Layers::TERRAIN:
            // 静的は「動的レイヤー」とだけ当てれば十分
            return bp == BroadPhaseLayers::DYNAMIC_BP;

        default:
            // 動く側 / キャラ / トリガー → 全部と当てる（静的＋動的）
            return true;
        }
    }
};


// ObjectLayerPairFilter
class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
    static constexpr uint32_t Key(JPH::ObjectLayer a, JPH::ObjectLayer b) {
        return (uint32_t(std::min(a, b)) << 16) | uint32_t(std::max(a, b));
    }

public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
    {
        using namespace Layers;

        uint32_t k = Key(a, b);

        // --- 1) Static 同士は不要 ---
        if (k == Key(NON_MOVING, NON_MOVING)
            || k == Key(NON_MOVING, TERRAIN)
            || k == Key(TERRAIN, TERRAIN))
        {
            return false;
        }

        // --- 2) Trigger の扱い ---
        // Trigger は「キャラ」と「動くもの」とだけ当てる（例）
        if (k == Key(TRIGGER, MOVING)
            || k == Key(TRIGGER, CHARACTER))
        {
            return true;
        }

        // --- 3) Character の扱い ---
        // キャラ vs 地形 / 静的 / 動的 → 当てる
        if (k == Key(CHARACTER, TERRAIN)
            || k == Key(CHARACTER, NON_MOVING)
            || k == Key(CHARACTER, MOVING))
        {
            return true;
        }

        // --- 4) それ以外は基本 true ---
        return true;
    }
};