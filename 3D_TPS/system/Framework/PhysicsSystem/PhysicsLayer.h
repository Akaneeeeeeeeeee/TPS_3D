#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <array>

// レイヤー定義
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;   // Static
    static constexpr JPH::ObjectLayer MOVING = 1;   // Dynamic
    static constexpr JPH::ObjectLayer CHARACTER = 2;   // Dynamic
    static constexpr JPH::ObjectLayer TRIGGER = 3;   // Dynamic(判定専用)
    static constexpr JPH::ObjectLayer TERRAIN = 4;   // Static
    static constexpr JPH::ObjectLayer NUM_LAYERS = 5;
}

namespace BroadPhaseLayers {
    static constexpr JPH::BroadPhaseLayer STATIC_BP = JPH::BroadPhaseLayer(0);
    static constexpr JPH::BroadPhaseLayer DYNAMIC_BP = JPH::BroadPhaseLayer(1);
    static constexpr uint32_t NUM = 2;
}

// BroadPhaseLayerInterface
class BPLayerInterface : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterface()
    {
        mMap.fill(BroadPhaseLayers::DYNAMIC_BP);   // まず全部動的に
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

    // 仮想クラスにさせないために仮定義
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

// ObjectVsBroadPhaseLayerFilter
// ぶつける or ぶつけない を定義
class ObjectVsBPLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer obj, JPH::BroadPhaseLayer bp) const override
    {
        switch (obj) {
        case Layers::NON_MOVING:
        case Layers::TERRAIN:
            return bp == BroadPhaseLayers::DYNAMIC_BP; // 静的は動的とだけ
        default:
            return true; // 動的側は両方と
        }
    }
};

// ObjectLayerPairFilter
class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
    //bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
    //{
    //    // 対称にする
    //    auto pair = [](JPH::ObjectLayer x, JPH::ObjectLayer y) { return (uint32_t(x) << 16) | y; };
    //    uint32_t p = pair(std::min(a, b), std::max(a, b));

    //    switch (p) {
    //        // Static vs Static は不要
    //        case (Layers::NON_MOVING << 16) | Layers::NON_MOVING: return false;
    //        case (Layers::TERRAIN << 16) | Layers::NON_MOVING: return false;
    //        case (Layers::TERRAIN << 16) | Layers::TERRAIN:    return false;

    //        // Trigger は全部と当てたい（例）
    //        case (Layers::TRIGGER << 16) | Layers::MOVING:     return true;
    //        case (Layers::TRIGGER << 16) | Layers::CHARACTER:  return true;

    //        // それ以外は基本 true
    //        default: return true;
    //    }
    //}
    static constexpr uint32_t Key(JPH::ObjectLayer a, JPH::ObjectLayer b) {
        return (uint32_t(std::min(a, b)) << 16) | uint32_t(std::max(a, b));
    }
public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
        switch (Key(a, b)) {
            // Static vs Static は不要
            case Key(Layers::NON_MOVING, Layers::NON_MOVING): return false;
            case Key(Layers::NON_MOVING, Layers::TERRAIN):    return false;
            case Key(Layers::TERRAIN, Layers::TERRAIN):    return false;

            // Trigger は全部と当てたい（例）
            case Key(Layers::TRIGGER, Layers::MOVING):     return true;
            case Key(Layers::TRIGGER, Layers::CHARACTER):  return true;

            // それ以外は基本 true
            default: return true;
        }
    }
};
