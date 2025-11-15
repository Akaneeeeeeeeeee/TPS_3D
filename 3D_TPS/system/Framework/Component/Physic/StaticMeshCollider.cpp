#include "StaticMeshCollider.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include "system/Framework/PhysicsSystem/PhysicsManager.h"
#include "system/Framework/PhysicsSystem/PhysicsLayer.h"
#include "renderer.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>


void StaticMeshCollider::Attach(EngineContext& context)
{
    PhysicsComponent::Attach(context);
}

void StaticMeshCollider::Init()
{
    if (!m_Physics) return;

    // SetMesh が呼ばれていない場合は作れない
    if (m_Positions.empty() || m_Triangles.empty())
        return;

    CreateBody(m_Physics->GetBodyInterface());
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

    for (auto& v : vertices)
        m_Positions.emplace_back(JPH::Float3(v.Position.x, v.Position.y, v.Position.z));

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
        verts.push_back(p); // JPH::Float3(x,y,z)

    JPH::Array<JPH::IndexedTriangle> tris;
    tris.reserve(m_Triangles.size());
    for (const auto& t : m_Triangles)
        tris.push_back(t); // JPH::IndexedTriangle(i0,i1,i2, material)

    // どのバージョンでも動く安全策：メンバに代入してから Create()
    JPH::MeshShapeSettings mesh_settings;
    mesh_settings.mTriangleVertices = std::move(verts);
    mesh_settings.mIndexedTriangles = std::move(tris);
    // 例: ランタイム優先
    // mesh_settings.mBuildQuality = JPH::MeshShapeSettings::EBuildQuality::FavorRuntimePerformance;

    // RefConst<Shape> を受け取る（生ポインタへのキャストは不要/不可）
    JPH::RefConst<JPH::Shape> shape = mesh_settings.Create().Get();
    m_Shape = shape; // メンバ保持

    JPH::BodyCreationSettings settings(
        m_Shape,                 // const Shape* に暗黙変換OK
        JPH::RVec3::sZero(),
        JPH::Quat::sIdentity(),
        JPH::EMotionType::Static,
        Layers::NON_MOVING
    );

    m_BodyID = bi.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
}

void StaticMeshCollider::Uninit()
{
    if (m_Physics && m_Physics->GetBodyInterface().IsAdded(m_BodyID))
    {
        m_Physics->GetBodyInterface().RemoveBody(m_BodyID);
    }
}

void StaticMeshCollider::Detach(EngineContext& context)
{
    m_Shape = nullptr;
    PhysicsComponent::Detach(context);
}
