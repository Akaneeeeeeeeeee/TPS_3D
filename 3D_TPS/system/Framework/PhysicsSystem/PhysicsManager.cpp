#include "PhysicsManager.h"
#include <Jolt/RegisterTypes.h>
#include "Framework/Component/Physic/PhysicsComponent.h"
#include "Framework/GameObject/GameObject.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#ifdef JPH_DEBUG_RENDERER
#include "Framework/PhysicsSystem/JoltDebugRendererDX11.h"
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Physics/Body/BodyManager.h>
#endif

namespace {
        constexpr float GRAVITY_SCALE = -98.0f;     // 重力加速度スケール
}

static void MyJoltTraceImpl(const char* inFMT, ...)
{
    char buffer[1024];

    va_list list;
    va_start(list, inFMT);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, inFMT, list);
    va_end(list);

	// デバッグログに出力
    printf("%s\n", buffer);
}

#ifdef JPH_ENABLE_ASSERTS
static bool MyJoltAssertFailedImpl(const char* inExpression,
    const char* inMessage,
    const char* inFile,
    UINT inLine)
{
    // デバッグログを出してブレークしたいならここで制御
    char buffer[1024];
    sprintf_s(buffer, "Jolt Assert Failed: %s\nMessage: %s\nFile: %s(%u)\n",
        inExpression, inMessage, inFile, inLine);
    OutputDebugStringA(buffer);

    // true を返すとブレークポイントを張ることを意味する（デフォルト仕様）
    return true;
}
#endif

PhysicsManager::PhysicsManager()
    : m_ObjectContactListener(*this),
	m_CharacterContactListener(*this)
{
}

PhysicsManager::~PhysicsManager()
{
#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
    m_DebugRenderer.reset(); // Ensures ~JoltDebugRendererDX11 -> ~DebugRenderer runs.
#endif
}

void PhysicsManager::Init(void)
{
    JPH::RegisterDefaultAllocator();
    // ★ここで差し替える
    JPH::Trace = MyJoltTraceImpl;
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
	// 衝突検知リスナーの登録
    m_System.SetContactListener(&m_ObjectContactListener);

#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
    m_DebugRenderer = std::make_unique<JoltDebugRendererDX11>();
    //JPH::DebugRenderer::sInstance = m_DebugRenderer.get();
#endif
}

void PhysicsManager::Update(const float deltaTime)
{
    m_System.Update(deltaTime, 1, m_TempAllocator.get(), m_JobSystem.get());
}

#ifdef _DEBUG
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
#endif

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


void PhysicsManager::OnCollisionEnter(GameObject& a, GameObject& b)
{
    a.OnCollisionEnter(b);
    b.OnCollisionEnter(a);
}

void PhysicsManager::OnCollisionStay(GameObject& a, GameObject& b)
{
    a.OnCollisionStay(b);
    b.OnCollisionStay(a);
}

void PhysicsManager::OnCollisionExit(GameObject& a, GameObject& b)
{
    a.OnCollisionExit(b);
    b.OnCollisionExit(a);
}

void PhysicsManager::OnTriggerEnter(GameObject& trigger, GameObject& other)
{
    trigger.OnTriggerEnter(other);
    other.OnTriggerEnter(trigger);
}

void PhysicsManager::OnTriggerStay(GameObject& trigger, GameObject& other)
{
    trigger.OnTriggerStay(other);
    other.OnTriggerStay(trigger);
}

void PhysicsManager::OnTriggerExit(GameObject& trigger, GameObject& other)
{
    trigger.OnTriggerExit(other);
    other.OnTriggerExit(trigger);
}

void PhysicsManager::OnCharacterCollisionEnter(GameObject& character, GameObject& other)
{
    character.OnCollisionCharacterEnter(other);
    other.OnCollisionCharacterEnter(character);
}

bool PhysicsManager::RaycastClosest(
    const Vector3& from, const Vector3& to,
    JPH::RayCastResult& outHit,
    const JPH::BodyID& ignoreBody) const
{
    const Vector3 d = to - from;
    const float lenSq = d.LengthSquared();
    if (lenSq < 1e-6f) return false;

    // RRayCast の origin は RVec3 にしておく（Joltの想定）
    const JPH::RVec3 origin(from.x, from.y, from.z);
    const JPH::Vec3  direction(d.x, d.y, d.z); // 長さ込み（from->to）

    const JPH::RRayCast ray(origin, direction);

    // Body フィルタ
    const JPH::IgnoreSingleBodyFilter bodyFilter(ignoreBody);

    const auto& npq = m_System.GetNarrowPhaseQuery();

    // ここは “遮蔽物レイ” 用にレイヤを合わせる（暫定で CHARACTER のままでも動く）
    auto bpFilter = m_System.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto objFilter = m_System.GetDefaultLayerFilter(Layers::CHARACTER);

    return npq.CastRay(ray, outHit, bpFilter, objFilter, bodyFilter);
}

bool PhysicsManager::IsOccluded(
    const Vector3& from, const Vector3& to,
    const JPH::BodyID& ignoreBody) const
{
    JPH::RayCastResult hit{};
    if (!RaycastClosest(from, to, hit, ignoreBody))
        return false;

    // 終点より手前で当たったら遮蔽
    return hit.mFraction < 0.999f;
}
