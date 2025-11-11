#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

// ÉåÉCÉÑÅ[íËã`
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
