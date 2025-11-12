#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "PhysicsLayer.h"
#include <Jolt/Core/JobSystemThreadPool.h>

class PhysicsComponent; // 前方宣言

/*
* @brief    物理演算マネージャー
* @detail   Jolt Physicsの物理演算システムを管理するクラス
* @remark   PhysicsComponentの派生クラス(Rigidbody,Collider系)が登録されている
* @remark   物理オブジェクトを管理し、物理演算の初期化と更新を行う
*/
class PhysicsManager
{
public:
    void Init(void);
    void Update(const float deltaTime);

    void Register(PhysicsComponent* rb);
	void UnRegister(PhysicsComponent* rb);

    JPH::PhysicsSystem& GetSystem() { return m_System; }
    JPH::BodyInterface& GetBodyInterface() { return m_System.GetBodyInterface(); }

private:
    JPH::PhysicsSystem m_System;
    std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
    std::vector<PhysicsComponent*> m_PhysicsObjects;
};
