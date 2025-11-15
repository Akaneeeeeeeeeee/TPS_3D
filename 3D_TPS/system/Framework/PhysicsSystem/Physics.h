#pragma once
#ifndef _DEBUG
#define JPH_DEBUG_RENDERER
#define JPH_PROFILE_ENABLED
#endif

#define JPH_FLOATING_POINT_EXCEPTIONS_ENABLED
#define JPH_OBJECT_STREAM
// ===== サードパーティ =====
#include <Jolt/Core/Core.h>

JPH_SUPPRESS_WARNING_PUSH
JPH_SUPPRESS_WARNINGS
JPH_MSVC_SUPPRESS_WARNING(6255)
JPH_MSVC_SUPPRESS_WARNING(6385)
JPH_MSVC_SUPPRESS_WARNING(6386)
JPH_MSVC_SUPPRESS_WARNING(26495)
JPH_MSVC_SUPPRESS_WARNING(26827)

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/StateRecorderImpl.h>

JPH_SUPPRESS_WARNING_POP