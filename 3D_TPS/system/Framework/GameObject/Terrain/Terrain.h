#pragma once
#include "Framework/GameObject/GameObject.h"

class CStaticMesh;
class CStaticMeshRenderer;
class CShader;
class StaticMeshCollider;
class IScene;

class Terrain : public GameObject
{
public:
    Terrain(ComponentFactory* factory,
        const uint64_t id,
        const std::string& name = "",
        const Tag tag = Tag::None,
        IScene* currentscene = nullptr,
        const Transform& transform = Transform::One())
        : GameObject(factory, id, name, tag, transform)
        , m_mesh(nullptr)
        , m_meshrenderer(nullptr)
        , m_shader(nullptr)
        , m_collider(nullptr)
        , m_ownerscene(currentscene)
    {
    }

    void Init() override;
    void Update(const float delta) override;
    void Draw() const override;
    void Uninit() override;

    void DebugImGui();
    // 地形の XZ 範囲
    const Vector3& GetXZMin() const { return m_XZMin; }
    const Vector3& GetXZMax() const { return m_XZMax; }

    // (x,z) の真下の地形表面の高さを取得する
    // 返り値: 高さが取れたら true
    bool GetWorldXZBounds(Vector3& outMin, Vector3& outMax) const;
    bool SampleHeight(float x, float z, float& outY) const;

private:
    CStaticMesh* m_mesh{};
    CStaticMeshRenderer* m_meshrenderer{};
    CShader* m_shader{};
    StaticMeshCollider* m_collider{};

    IScene* m_ownerscene = nullptr;
    Vector3 m_XZMin = Vector3::Zero;
    Vector3 m_XZMax = Vector3::Zero;
};
