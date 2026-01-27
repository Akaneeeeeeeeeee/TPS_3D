#include "StaticMeshCollider.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/Framework/EngineSystem/EngineSystem.h"
#include "system/Framework/PhysicsSystem/PhysicsManager.h"
#include "system/Framework/PhysicsSystem/PhysicsLayer.h"
#include "renderer.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include "system/CStaticMesh.h"

#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>

// ==============================
// Debug-only log (Releaseでは出ない)
// ==============================
#if defined(_DEBUG)
#define JOLT_LOG(...) JPH::Trace(__VA_ARGS__)
#else
#define JOLT_LOG(...) ((void)0)
#endif

namespace
{
    // このコライダの BodyID だけを許可する BodyFilter
    class SingleBodyFilter final : public JPH::BodyFilter
    {
    public:
        explicit SingleBodyFilter(JPH::BodyID id) : mTarget(id) {}

        bool ShouldCollide(const JPH::BodyID& inBodyID) const override
        {
            return inBodyID == mTarget;
        }

    private:
        JPH::BodyID mTarget;
    };
}

bool StaticMeshCollider::SampleHeight(float x, float z, float& outY) const
{
    if (!m_Physics || m_BodyID.IsInvalid()) { return false; }

    using namespace JPH;

    auto& system = m_Physics->GetSystem();
    auto& npq = system.GetNarrowPhaseQuery();

    // 上から下へ長いレイを飛ばす
    const float CAST_HEIGHT = 10000.0f;
    const float MAX_DIST = 20000.0f;

    RVec3 origin(x, CAST_HEIGHT, z);
    Vec3  dir(0.0f, -1.0f, 0.0f);

    RRayCast ray(origin, dir * MAX_DIST);
    RayCastResult hit;

    // Layers::TERRAIN「TERRAIN と衝突する側」のレイヤーにする（例: CHARACTER）
    auto bpFilter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto objFilter = system.GetDefaultLayerFilter(Layers::CHARACTER);

    // この StaticMeshCollider の Body だけに絞る
    SingleBodyFilter bodyFilter(m_BodyID);

    if (!npq.CastRay(ray, hit, bpFilter, objFilter, bodyFilter))
    {
        JOLT_LOG("SampleHeight: CastRay no hit (CHARACTER filter).");
        return false;
    }

    float dist = hit.mFraction * MAX_DIST;
    outY = CAST_HEIGHT - dist;
    return true;
}

void StaticMeshCollider::Attach(EngineServices& context)
{
    PhysicsComponent::Attach(context);
}

void StaticMeshCollider::Init()
{
    if (!m_Physics) { return; }

    // SetMesh が呼ばれていない場合は作れない
    if (m_Positions.empty() || m_Triangles.empty()) { return; }

    CreateBody(m_Physics->GetBodyInterface());
    JOLT_LOG("StaticMeshCollider: BodyID = %u", m_BodyID.GetIndex());
}

void StaticMeshCollider::SetMesh(const CStaticMesh& mesh)
{
    // 既存 Body を消す
    if (m_Physics && !m_BodyID.IsInvalid())
    {
        auto& bi = m_Physics->GetBodyInterface();
        if (bi.IsAdded(m_BodyID))
        {
            bi.RemoveBody(m_BodyID);
        }
        bi.DestroyBody(m_BodyID);
        m_BodyID = JPH::BodyID();
    }

    // オーナーのスケールを取得（必要なら頂点に適用する）
    Vector3 scale = m_pOwner ? m_pOwner->GetScale() : Vector3::One;

    // 頂点配列・インデックス配列・サブセットを取得
    const auto& vertices = mesh.GetVertices();   // フラット配列
    const auto& indices = mesh.GetIndices();    // フラット配列
    const auto& subsets = mesh.GetSubsets();    // SUBSET 配列

    // 位置リスト
    m_Positions.clear();
    m_Positions.reserve(vertices.size());

    // 頂点位置をスケール適用して登録
    for (auto& v : vertices)
    {
        Vector3 p = v.Position;
        p.x *= scale.x;
        p.y *= scale.y;
        p.z *= scale.z;

        m_Positions.emplace_back(JPH::Float3(p.x, p.y, p.z));
    }

    // 三角形リスト
    m_Triangles.clear();

    for (const auto& sub : subsets)
    {
        const uint32_t index_begin = sub.IndexBase;
        const uint32_t index_end = sub.IndexBase + sub.IndexNum;
        const uint32_t vbase = sub.VertexBase;

        // サブセット内のインデックスは「ローカル頂点番号」なので vbase を足す
        for (uint32_t i = index_begin; i + 2 < index_end; i += 3)
        {
            uint32_t i0 = indices[i + 0] + vbase;
            uint32_t i1 = indices[i + 1] + vbase;
            uint32_t i2 = indices[i + 2] + vbase;

            JPH::IndexedTriangle tri;
            tri.mIdx[0] = i0;
            tri.mIdx[1] = i1;
            tri.mIdx[2] = i2;
            tri.mMaterialIndex = 0;

            m_Triangles.emplace_back(tri);
        }
    }
}

void StaticMeshCollider::SetMesh(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices)
{
    // 既存の地形を削除（再生成対応）
    if (m_Physics && !m_BodyID.IsInvalid())
    {
        auto& bi = m_Physics->GetBodyInterface();
        if (bi.IsAdded(m_BodyID)) bi.RemoveBody(m_BodyID);
        m_BodyID = JPH::BodyID();
    }

    // 位置リスト作成
    m_Positions.clear();
    m_Positions.reserve(vertices.size());

    // オーナーのスケールを取得（必要なら頂点に適用する）
    Vector3 scale = m_pOwner ? m_pOwner->GetScale() : Vector3::One;

    // 頂点位置をスケール適用して登録
    for (auto& v : vertices)
    {
        Vector3 p = v.Position;
        p.x *= scale.x;
        p.y *= scale.y;
        p.z *= scale.z;

        m_Positions.emplace_back(JPH::Float3(p.x, p.y, p.z));
    }

    // 三角形リスト作成
    m_Triangles.clear();
    size_t triCount = indices.size() / 3;
    m_Triangles.reserve(triCount);

    for (size_t i = 0; i < triCount; ++i)
    {
        JPH::IndexedTriangle t;
        t.mIdx[0] = indices[i * 3 + 0];
        t.mIdx[1] = indices[i * 3 + 1];
        t.mIdx[2] = indices[i * 3 + 2];
        t.mMaterialIndex = 0;

        m_Triangles.emplace_back(t);
    }

    // SetMesh の後で Init() が呼ばれていれば Body 作成も可能
}

void StaticMeshCollider::CreateBody(JPH::BodyInterface& bi)
{
    using namespace JPH;

    // std::vector → JPH::Array に詰め替え
    JPH::Array<JPH::Float3> verts;
    verts.reserve(m_Positions.size());
    for (const auto& p : m_Positions)
    {
        verts.push_back(p); // JPH::Float3(x,y,z)
    }

    JPH::Array<JPH::IndexedTriangle> tris;
    tris.reserve(m_Triangles.size());
    for (const auto& t : m_Triangles)
    {
        tris.push_back(t); // JPH::IndexedTriangle(i0,i1,i2, material)
    }

    // どのバージョンでも動く安全策：メンバに代入してから Create()
    MeshShapeSettings mesh_settings;
    mesh_settings.mTriangleVertices = std::move(verts);
    mesh_settings.mIndexedTriangles = std::move(tris);
    // メッシュデータを検証、縮退した三角形を削除する
    mesh_settings.Sanitize();

    // ----- Shape を作成 -----
    auto result = mesh_settings.Create();          // ShapeSettings::ShapeResult

    if (result.HasError())
    {
        // ここでエラーメッセージをログに出しておくと原因が分かりやすい
        JOLT_LOG("MeshShape Create error: %s", result.GetError().c_str());
        m_Shape = nullptr;
        return;
    }

    ShapeRefC shape = result.Get();                // ShapeRefC = RefConst<Shape>
    m_Shape = shape;

    // オーナーの位置を使うならここで変換
    Vector3 pos = m_pOwner ? m_pOwner->GetPosition() : Vector3::Zero;
    RVec3  jpos(pos.x, pos.y, pos.z);

    BodyCreationSettings settings(
        m_Shape,                 // const Shape* へ暗黙変換
        jpos,
        Quat::sIdentity(),
        EMotionType::Static,
        Layers::TERRAIN
    );

    // GameObject の ID を UserData に入れる
    settings.mUserData = m_pOwner ? m_pOwner->GetID() : 0;

    m_BodyID = bi.CreateAndAddBody(settings, EActivation::DontActivate);
}

void StaticMeshCollider::Uninit()
{
    if (m_Physics && !m_BodyID.IsInvalid())
    {
        auto& bi = m_Physics->GetBodyInterface();

        if (bi.IsAdded(m_BodyID))
        {
            bi.RemoveBody(m_BodyID);
        }
        bi.DestroyBody(m_BodyID);

        m_BodyID = JPH::BodyID();
    }

    m_Shape = nullptr; // 使ってるなら
    m_Physics = nullptr;
}

void StaticMeshCollider::Detach(void)
{
    m_Shape = nullptr;
    PhysicsComponent::Detach();
}

bool StaticMeshCollider::GetWorldXZBounds(Vector3& outMin, Vector3& outMax) const
{
    if (!m_Physics || m_BodyID.IsInvalid()) { return false; }

    using namespace JPH;

    // PhysicsSystem を取得
    auto& system = m_Physics->GetSystem();

    // BodyLockInterfaceNoLock はコピー不可なので、参照で取得する
    auto& lock_interface = system.GetBodyLockInterfaceNoLock();

    // 読み取りロック
    BodyLockRead lock(lock_interface, m_BodyID);
    if (!lock.Succeeded()) { return false; }

    const Body& body = lock.GetBody();

    // ここで world AABB を取得
    const AABox& aabb = body.GetWorldSpaceBounds();

    outMin = Vector3(aabb.mMin.GetX(), aabb.mMin.GetY(), aabb.mMin.GetZ());
    outMax = Vector3(aabb.mMax.GetX(), aabb.mMax.GetY(), aabb.mMax.GetZ());
    return true;
}
