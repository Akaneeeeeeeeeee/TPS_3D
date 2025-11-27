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

private:
    CStaticMesh* m_mesh{};
    CStaticMeshRenderer* m_meshrenderer{};
    CShader* m_shader{};
    StaticMeshCollider* m_collider{};

    IScene* m_ownerscene = nullptr;
};
