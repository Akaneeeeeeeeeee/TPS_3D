#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "PhysicsLayer.h"
#include <Jolt/Core/JobSystemThreadPool.h>

class PhysicsComponent; // ëOï˚êÈåæ

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
