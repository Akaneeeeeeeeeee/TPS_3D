#include "PhysicsManager.h"
#include <Jolt/RegisterTypes.h>
#include "Framework/Component/Physic/PhysicsComponent.h"
#ifdef JPH_DEBUG_RENDERER
#include "Framework/PhysicsSystem/JoltDebugRendererDX11.h"
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Physics/Body/BodyManager.h>
#endif

namespace {
        constexpr float GRAVITY_SCALE = -98.0f;     // 重力加速度スケール
}

PhysicsManager::~PhysicsManager()
{
#ifdef JPH_DEBUG_RENDERER
    m_DebugRenderer.reset(); // Ensures ~JoltDebugRendererDX11 -> ~DebugRenderer runs.
#endif
}

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
    m_System.SetGravity(JPH::Vec3(0.0f, GRAVITY_SCALE, 0.0f));

    m_TempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    m_JobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);

#ifdef JPH_DEBUG_RENDERER
    m_DebugRenderer = std::make_unique<JoltDebugRendererDX11>();
    //JPH::DebugRenderer::sInstance = m_DebugRenderer.get();
#endif
}

void PhysicsManager::Update(const float deltaTime)
{
    m_System.Update(deltaTime, 1, m_TempAllocator.get(), m_JobSystem.get());
}

void PhysicsManager::DebugDraw(void)
{
#ifdef JPH_DEBUG_RENDERER
    if (!m_DebugRenderer) { return; }


    m_DebugRenderer->NextFrame();

    JPH::BodyManager::DrawSettings settings;
    settings.mDrawShape = true;
    settings.mDrawShapeWireframe = true;
    settings.mDrawBoundingBox = false;
    settings.mDrawVelocity = false;

    m_System.DrawBodies(settings, m_DebugRenderer.get());


    //// ---- 竭 繝舌ャ繝・幕蟋具ｼ医け繝ｪ繧｢ & VP 陦悟・險ｭ螳夲ｼ・----
    //m_DebugRenderer->Begin(vp);

    //// ---- 竭｡ Jolt 縺ｮ繝・ヰ繝・げ謠冗判 ----
    //m_DebugRenderer->NextFrame(); // Jolt 縺ｮ蜀・Κ迥ｶ諷区峩譁ｰ・医％繧後・蠢・ｦ・ｼ・

    //JPH::BodyManager::DrawSettings settings;
    //settings.mDrawShape = true;
    //settings.mDrawShapeWireframe = true;
    //settings.mDrawBoundingBox = false;
    //settings.mDrawVelocity = false;

    //// 縺薙％縺ｧ Jolt 縺・DrawLine 繧貞､ｧ驥上↓蜻ｼ縺ｶ・医′謠冗判縺ｯ縺励↑縺・ｼ・
    //m_System.DrawBodies(settings, m_DebugRenderer.get());

    //// ---- 竭｢ 繝舌ャ繝∫ｵゆｺ・ｼ・lushLines・・----
    //m_DebugRenderer->End();

#endif // JPH_DEBUG_RENDERER
}

void PhysicsManager::Register(PhysicsComponent* component)
{
    if (component)
    {
        m_PhysicsObjects.push_back(component);
        //component->CreateBody(m_System.GetBodyInterface());
    }
}

void PhysicsManager::UnRegister(PhysicsComponent* component)
{
    component->DestroyBody(m_System.GetBodyInterface());
}

