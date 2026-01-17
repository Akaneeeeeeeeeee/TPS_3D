#pragma once
#include "PhysicsComponent.h"

// 前方宣言
class CStaticMesh;

/*
* @brief	メッシュコライダーコンポーネント
* @detail	任意のメッシュ形状のコライダーを提供するコンポーネント
* @remark	地形は一つしかないため、Rigidbodyは使用せず、このコライダー内でBodyを生成する。
* @auther	赤根 和樹
* @date     2025/11/11
*/
class StaticMeshCollider : public PhysicsComponent
{
public:
	DECLARE_COMPONENT_TYPE(StaticMeshCollider, PhysicsComponent)
    StaticMeshCollider() = default;
    ~StaticMeshCollider() noexcept override = default;

    void CreateBody(JPH::BodyInterface& bi) override;

    // CStaticMesh 版
    void SetMesh(const CStaticMesh& mesh);

    void Init(void) override;
    void Update(const float deltaTime) override {}
    void Uninit(void) override;

    void Attach(EngineServices& context) override;
    void Detach(void) override;

    // 形状を取得
    JPH::RefConst<JPH::Shape> GetShape(void) const override { return JPH::RefConst<JPH::Shape>(m_Shape); }
    bool IsCollider() const noexcept override { return true; }

    // メッシュデータ を渡してもらう
    void SetMesh(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices) override;

    // 地形クエリ用の API
    // XZ の AABB を返す（ワールド座標）
    bool GetWorldXZBounds(Vector3& outMin, Vector3& outMax) const;

    // (x, z) の真下の地形表面 Y を取得
    bool SampleHeight(float x, float z, float& outY) const;
private:
    std::vector<JPH::Float3>            m_Positions;
    std::vector<JPH::IndexedTriangle>   m_Triangles;
    JPH::RefConst<JPH::Shape>           m_Shape = nullptr;
};
