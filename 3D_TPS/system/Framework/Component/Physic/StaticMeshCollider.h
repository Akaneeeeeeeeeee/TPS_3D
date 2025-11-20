#pragma once
#include "PhysicsComponent.h"

/*
* @brief	メッシュコライダーコンポーネント
* @detail	任意のメッシュ形状のコライダーを提供するコンポーネント
* @remark	物理演算を行うためには Rigidbody コンポーネントと組み合わせて使用する必要がある
* @remark   !!
* @auther	赤根 和樹
* @date     2025/11/11
*/
class StaticMeshCollider : public PhysicsComponent
{
public:
public:
    StaticMeshCollider() = default;
    ~StaticMeshCollider() noexcept override = default;

    void CreateBody(JPH::BodyInterface& bi) override;

    void Init(void) override;
    void Update(const float deltaTime) override {}
    void Uninit(void) override;

    void Attach(EngineContext& context) override;
    void Detach(void) override;

    // 形状を取得
    JPH::RefConst<JPH::Shape> GetShape(void) const override { return JPH::RefConst<JPH::Shape>(m_Shape); }
    bool IsCollider() const noexcept override { return true; }

    // メッシュデータ を渡してもらう
    void SetMesh(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices) override;


private:
    std::vector<JPH::Float3>            m_Positions;
    std::vector<JPH::IndexedTriangle>   m_Triangles;
    JPH::RefConst<JPH::Shape>           m_Shape = nullptr;
};
