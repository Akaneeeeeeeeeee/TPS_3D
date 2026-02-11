#include "PhysicsManager.h"
#include <Jolt/RegisterTypes.h>
#include "Framework/Component/Physic/PhysicsComponent.h"
#include "Framework/ObjectManager/ObjectManager.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#ifdef JPH_DEBUG_RENDERER
#include "Framework/PhysicsSystem/JoltDebugRendererDX11.h"
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Physics/Body/BodyManager.h>
#endif

namespace {
        constexpr float GRAVITY_SCALE = -980.0f;     // 重力加速度スケール
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
}

void PhysicsManager::Init(void)
{
    JPH::RegisterDefaultAllocator();
    // ここで差し替える
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

void PhysicsManager::Uninit()
{
    // 物理更新中に呼ばない前提（マルチスレッド更新してるなら停止/同期してから）
    m_System.SetContactListener(nullptr);

    // 登録済みコンポーネントのボディを必ず破棄
   /* {
        auto& bi = m_System.GetBodyInterface();
        for (auto* comp : m_PhysicsObjects)
        {
            if (comp)
            {
                comp->DestroyBody(bi);
            }
        }
        m_PhysicsObjects.clear();
    }*/

    // キューも掃除
    {
        std::lock_guard<std::mutex> lk(m_EventMtx);
        m_EventQueue.clear();
    }

#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG) && defined(JPH_DEBUG_RENDERER)
    m_DebugRenderer.reset();
#endif

    m_JobSystem.reset();
    m_TempAllocator.reset();

    // ---- Jolt のグローバル後始末 ----
    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

#ifdef _DEBUG

void PhysicsManager::SetCameraManager(class CameraManager* cameraManager)
{
    m_CameraManager = cameraManager;
}

void PhysicsManager::DebugDraw(void)
{
#ifdef JPH_DEBUG_RENDERER
    if (!m_DebugRenderer) { return; }

	DirectX::XMMATRIX vp = m_CameraManager->GetMain()->GetProjMatrix() * m_CameraManager->GetMain()->GetViewMatrix();
	m_DebugRenderer->Begin(vp);
    m_DebugRenderer->NextFrame();

    JPH::BodyManager::DrawSettings settings;
    settings.mDrawShape = true;
    settings.mDrawShapeWireframe = true;
    settings.mDrawBoundingBox = false;
    settings.mDrawVelocity = false;

    m_System.DrawBodies(settings, m_DebugRenderer.get());
	m_DebugRenderer->End();


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
    if (!component) return;

    // 配列からも外す（残ると二重Destroyなどの温床）
    m_PhysicsObjects.erase(
        std::remove(m_PhysicsObjects.begin(), m_PhysicsObjects.end(), component),
        m_PhysicsObjects.end()
    );

    component->DestroyBody(m_System.GetBodyInterface());
}
void PhysicsManager::Update(const float deltaTime)
{
    m_System.Update(deltaTime, 1, m_TempAllocator.get(), m_JobSystem.get());
    // ここでは Dispatch しない（呼び出し側＝メインスレッドが Update直後に呼ぶ）
}

void PhysicsManager::EnqueueEvent(CollisionEvent::Type type, uint64_t aId, uint64_t bId)
{
    // 物理スレッドから呼ばれる。GameObject には触らず、軽い処理だけ。
    if (aId == 0 || bId == 0) return;

    std::lock_guard<std::mutex> lk(m_EventMtx);
    m_EventQueue.push_back(CollisionEvent{ type, aId, bId });
}

void PhysicsManager::DispatchCollisionEvents(void)
{
    // 必ずメインスレッドで呼ぶ（Physics.Updateの後）
    std::vector<CollisionEvent> events;
    {
        std::lock_guard<std::mutex> lk(m_EventMtx);
        events.swap(m_EventQueue);
    }

    if (!m_ObjectManager) return;

    for (const auto& ev : events)
    {
        GameObject* a = m_ObjectManager->GetObjectByID<GameObject>(ev.aId);
        GameObject* b = m_ObjectManager->GetObjectByID<GameObject>(ev.bId);

        // Destroy済み/存在しないなら呼ばない（ここで落ちないようにする）
        if (!a || !b) continue;
        if (a->IsDestroy() || b->IsDestroy()) continue;

        switch (ev.type)
        {
        case CollisionEvent::Type::CollisionEnter:
            OnCollisionEnter(*a, *b);
            break;
        case CollisionEvent::Type::CollisionExit:
            OnCollisionExit(*a, *b);
            break;
        case CollisionEvent::Type::TriggerEnter:
            OnTriggerEnter(*a, *b);
            break;
        case CollisionEvent::Type::TriggerExit:
            OnTriggerExit(*a, *b);
            break;
        case CollisionEvent::Type::CharacterEnter:
            OnCharacterCollisionEnter(*a, *b);
            break;
        case CollisionEvent::Type::CharacterExit:
            OnCharacterCollisionExit(*a, *b);
            break;
        default:
            break;
        }
    }
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

void PhysicsManager::OnCharacterCollisionExit(GameObject& a, GameObject& b)
{
    a.OnCollisionCharacterExit(b);
    b.OnCollisionCharacterExit(a);
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
