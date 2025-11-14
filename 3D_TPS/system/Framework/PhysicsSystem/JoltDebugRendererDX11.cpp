#include "JoltDebugRendererDX11.h"

#include "system/LineDrawer.h"
#include "commontypes.h"

#include <Jolt/Core/Color.h>

#include <limits>

namespace
{
    Color ToColor(JPH::ColorArg src)
    {
        constexpr float inv255 = 1.0f / 255.0f;
        return Color(src.r * inv255, src.g * inv255, src.b * inv255, src.a * inv255);
    }

    Vector3 ToVector3(JPH::RVec3Arg v)
    {
        return Vector3(static_cast<float>(v.GetX()), static_cast<float>(v.GetY()), static_cast<float>(v.GetZ()));
    }

    std::once_flag g_init_flag;
}

void JoltDebugRendererDX11::EnsureResources()
{
    std::call_once(g_init_flag, []() {
        LineDrawerInit();
        SetLineWidth(1.0f);
    });
}

void JoltDebugRendererDX11::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    EnsureResources();

    Vector3 start = ToVector3(inFrom);
    Vector3 end = ToVector3(inTo);
    Vector3 direction = end - start;
    float length = direction.Length();
    if (length <= std::numeric_limits<float>::epsilon())
    {
        return;
    }

    direction /= length;
    SetLineWidth(1.0f);
    LineDrawerDraw(length, start, direction, ToColor(inColor));
}

void JoltDebugRendererDX11::DrawText3D(JPH::RVec3Arg, const std::string_view &, JPH::ColorArg, float)
{
    // Text drawing is not required for the current collider debug rendering use-case.
}

