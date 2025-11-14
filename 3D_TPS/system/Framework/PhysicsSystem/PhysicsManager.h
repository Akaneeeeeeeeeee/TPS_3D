#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "PhysicsLayer.h"
#include <Jolt/Core/JobSystemThreadPool.h>
#include <memory>
#include <vector>
#include "JoltDebugRendererDX11.h"

class PhysicsComponent; // 前方宣言
#ifdef JPH_DEBUG_RENDERER
class JoltDebugRendererDX11;
#endif

/*
* @brief    物理マネージャ
* @detail   Jolt Physicsの管理クラス
* @remark   PhysicsComponentの派生(Rigidbody,Colliderなど)が登録対象
* @remark   エンジン全体で共有するシングルトン的な扱いを想定
*/
class PhysicsManager
{
public:
    ~PhysicsManager();
    void Init(void);
    void Update(const float deltaTime);

    void DebugDraw();

    void Register(PhysicsComponent* rb);
    void UnRegister(PhysicsComponent* rb);

    JPH::PhysicsSystem& GetSystem() { return m_System; }
    JPH::BodyInterface& GetBodyInterface() { return m_System.GetBodyInterface(); }

private:
    JPH::PhysicsSystem m_System;
    std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
    std::vector<PhysicsComponent*> m_PhysicsObjects;
#ifdef JPH_DEBUG_RENDERER
    std::unique_ptr<JoltDebugRendererDX11> m_DebugRenderer;
#endif
};

