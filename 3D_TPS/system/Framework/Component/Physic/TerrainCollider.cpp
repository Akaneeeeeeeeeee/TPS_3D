#include "TerrainCollider.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include "system/Framework/PhysicsSystem/PhysicsManager.h"
#include "system/Framework/PhysicsSystem/PhysicsLayer.h"
#include "renderer.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>


void TerrainCollider::Attach(EngineContext& context)
{
    PhysicsComponent::Attach(context);
}

void TerrainCollider::Init()
{
    if (!m_Physics) { return; }

    auto& bi = m_Physics->GetBodyInterface();
    CreateBody(bi);
}

void TerrainCollider::SetMesh(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices)
{
    m_Positions.clear();
    m_Triangles.clear();

    m_Positions.reserve(vertices.size());
    for (const auto& v : vertices)
        m_Positions.emplace_back(JPH::Float3(v.Position.x, v.Position.y, v.Position.z));

    // 3つずつで三角形
    const size_t tri_count = indices.size() / 3;
    m_Triangles.reserve(tri_count);
    for (size_t i = 0; i < tri_count; ++i) {
        JPH::IndexedTriangle t;
        t.mIdx[0] = (uint32_t)indices[i * 3 + 0];
        t.mIdx[1] = (uint32_t)indices[i * 3 + 1];
        t.mIdx[2] = (uint32_t)indices[i * 3 + 2];
        t.mMaterialIndex = 0;
        m_Triangles.emplace_back(t);
    }
}

void TerrainCollider::CreateBody(JPH::BodyInterface& bi)
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

void TerrainCollider::Uninit()
{
    if (m_Physics && m_Physics->GetBodyInterface().IsAdded(m_BodyID))
    {
        m_Physics->GetBodyInterface().RemoveBody(m_BodyID);
    }
}

void TerrainCollider::Detach(EngineContext& context)
{
    m_Shape = nullptr;
    PhysicsComponent::Detach(context);
}
