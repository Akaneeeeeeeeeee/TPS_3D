#pragma once
#include	<wrl/client.h>
#include	<cstdint>
#include	<numbers>
#include	<SimpleMath.h>

// 基本型のエイリアス
using Vector4 = DirectX::SimpleMath::Vector4;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector2 = DirectX::SimpleMath::Vector2;

using Matrix4x4 = DirectX::SimpleMath::Matrix;

using Color = DirectX::SimpleMath::Color;

using Quaternion = DirectX::SimpleMath::Quaternion;

using Microsoft::WRL::ComPtr;

constexpr float PI = std::numbers::pi_v<float>;

static bool IsInCone(
    const Vector3& origin,
    const Vector3& forwardNormalized,
    float range,
    float cosHalfAngle,
    const Vector3& point)
{
    Vector3 to = point - origin;
    float dist = to.Length();
    if (dist <= 1e-4f || dist > range) { return false; }

    Vector3 dir = to / dist; // normalize
    float c = forwardNormalized.Dot(dir);
    return c >= cosHalfAngle;
}
