#pragma once

#include "Framework/PhysicsSystem/Physics.h"

#include <Jolt/Renderer/DebugRendererSimple.h>

#include <mutex>
#include <string_view>

class JoltDebugRendererDX11 final : public JPH::DebugRendererSimple
{
public:
    JoltDebugRendererDX11() = default;
    ~JoltDebugRendererDX11() override = default;

    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override;

private:
    static void EnsureResources();
};

