#pragma once
#include "system/Framework/Component/Physic/PhysicsComponent.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

struct VERTEX_3D;

/*
* @brief	Terrainコライダーコンポーネント
* @detail	地形のコライダーを提供するコンポーネント
* @remark	地形は一つしかないため、Rigidbodyは使用せず、このコライダー内でBodyを生成する。
* @auther	赤根 和樹
* @date     2025/11/11
*/
class TerrainCollider : public PhysicsComponent
{
public:
    TerrainCollider() = default;
    ~TerrainCollider() noexcept override = default;

    void CreateBody(JPH::BodyInterface& bi) override;

    void Init(void) override;
    void Update(const float deltaTime) override {}
    void Uninit(void) override;

    void Attach(EngineServices& context) override;
    void Detach(void) override;

	// 形状を取得
    JPH::RefConst<JPH::Shape> GetShape(void) const override { return JPH::RefConst<JPH::Shape>(m_Shape); }
    bool IsCollider() const noexcept override { return true; }

    // Terrain から渡してもらう
    void SetMesh(const std::vector<VERTEX_3D>& vertices,const std::vector<uint32_t>& indices) override;

private:
    std::vector<JPH::Float3>            m_Positions;
    std::vector<JPH::IndexedTriangle>   m_Triangles;
    JPH::RefConst<JPH::Shape>           m_Shape = nullptr;
};
