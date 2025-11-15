#include "PhysicsManager.h"
#include <Jolt/RegisterTypes.h>
#include "Framework/Component/Physic/PhysicsComponent.h"
#ifdef JPH_DEBUG_RENDERER
#include "Framework/PhysicsSystem/JoltDebugRendererDX11.h"
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Physics/Body/BodyManager.h>
#endif

namespace {
        constexpr float GRAVITY_SCALE = -750.0f;     // 重力加速度スケール
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


    //// ---- ① バッチ開始（クリア & VP 行列設定） ----
    //m_DebugRenderer->Begin(vp);

    //// ---- ② Jolt のデバッグ描画 ----
    //m_DebugRenderer->NextFrame(); // Jolt の内部状態更新（これは必要）

    //JPH::BodyManager::DrawSettings settings;
    //settings.mDrawShape = true;
    //settings.mDrawShapeWireframe = true;
    //settings.mDrawBoundingBox = false;
    //settings.mDrawVelocity = false;

    //// ここで Jolt が DrawLine を大量に呼ぶ（が描画はしない）
    //m_System.DrawBodies(settings, m_DebugRenderer.get());

    //// ---- ③ バッチ終了（FlushLines） ----
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

