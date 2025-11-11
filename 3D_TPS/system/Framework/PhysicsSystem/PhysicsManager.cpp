#include "PhysicsManager.h"
#include <Jolt/RegisterTypes.h>
#include "Framework/Component/Physic/PhysicsComponent.h"

void PhysicsManager::Init()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    static BPLayerInterface broadPhaseLayerInterface;
    static ObjectVsBPLayerFilter objectVsBPLayerFilter;
    static ObjectLayerPairFilter objectLayerPairFilter;

    m_System.Init(
        10000, 0, 65536, 10240,
        broadPhaseLayerInterface,
        objectVsBPLayerFilter,
        objectLayerPairFilter
    );

    m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);
}

void PhysicsManager::Update(const float deltaTime)
{
    m_System.Update(deltaTime, 1, m_TempAllocator.get(), m_JobSystem.get());
}

void PhysicsManager::Register(PhysicsComponent* component)
{
    if (component)
    {
        m_PhysicsObjects.push_back(component);
        component->CreateBody(m_System.GetBodyInterface());
    }
}

void PhysicsManager::UnRegister(PhysicsComponent* component)
{
	component->DestroyBody(m_System.GetBodyInterface());
}