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

// -------------------------
// BodyFilter: キャラとトリガーを除外
// -------------------------
class AvoidCharAndTriggerBodyFilter : public JPH::BodyFilter
{
public:
    explicit AvoidCharAndTriggerBodyFilter(JPH::PhysicsSystem& system)
        : m_System(system)
    {
    }

    bool ShouldCollide(const JPH::BodyID& bodyID) const override
    {
        JPH::BodyLockRead lock(m_System.GetBodyLockInterface(), bodyID);
        if (!lock.Succeeded())
            return false;

        return ShouldCollideLocked(lock.GetBody());
    }

    bool ShouldCollideLocked(const JPH::Body& body) const override
    {
        auto layer = body.GetObjectLayer();

        // キャラとトリガーはレイキャストの対象外
        if (layer == Layers::CHARACTER || layer == Layers::TRIGGER)
            return false;

        return true;
    }

private:
    JPH::PhysicsSystem& m_System;
};


class EscapeBodyFilter : public JPH::BodyFilter
{
public:
    EscapeBodyFilter(const JPH::PhysicsSystem& system)
        : mSystem(system) {
    }

    bool ShouldCollide(const JPH::BodyID& body_id) const override
    {
        JPH::BodyLockRead lock(mSystem.GetBodyLockInterface(), body_id);
        if (!lock.Succeeded()) return false;

        const JPH::Body& body = lock.GetBody();
        auto layer = body.GetObjectLayer();

        // 自キャラとトリガーは従来通り無視
        if (layer == Layers::CHARACTER || layer == Layers::TRIGGER)
            return false;

        // Terrain レイヤはスタック解除の判定から除外する
        if (layer == Layers::TERRAIN)   // ← ここを layer=4 に対応させる
            return false;

        // それ以外（壁・岩・障害物）は判定対象
        return true;
    }

private:
    const JPH::PhysicsSystem& mSystem;
};


class OcclusionObjectLayerFilter : public JPH::ObjectLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer layer) const override
    {
        // 音を遮るものだけ拾う
        return layer == Layers::NON_MOVING
            || layer == Layers::MOVING
            || layer == Layers::TERRAIN;
    }
};

class OcclusionBroadPhaseLayerFilter : public JPH::BroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::BroadPhaseLayer) const override { return true; } // STATIC/DYNAMIC両方
};
