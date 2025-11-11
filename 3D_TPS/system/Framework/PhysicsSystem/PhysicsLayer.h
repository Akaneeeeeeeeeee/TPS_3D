#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

// レイヤー定義
namespace Layers
{
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
}

// BroadPhaseLayerInterface
class BPLayerInterface : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterface()
    {
        mObjectToBroadPhase[Layers::NON_MOVING] = JPH::BroadPhaseLayer(0);
    }

    uint32_t GetNumBroadPhaseLayers() const override { return 1; }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        return mObjectToBroadPhase[layer];
    }

    // 仮想クラスにさせないために仮定義
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        /*switch (inLayer)
        {
        case 0: return "NON_MOVING";
        default: return "Unknown";
        }*/
		return "Layer";
    }

private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[1];
};

// ObjectVsBroadPhaseLayerFilter
class ObjectVsBPLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override
    {
        return true;
    }
};

// ObjectLayerPairFilter
class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const override
    {
        return true;
    }
};
