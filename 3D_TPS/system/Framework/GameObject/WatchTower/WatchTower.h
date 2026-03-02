#pragma once
#include "Framework/GameObject/GameObject.h"

class SpotLightComponent;
class SearchLightControllerComponent;

class WatchTower : public GameObject
{
public:
    WatchTower(ComponentFactory* factory,
        const uint64_t id,
        const std::string& name = "WatchTower",
        const Tag tag = Tag::Object,
        const Transform& transform = Transform::One())
        : GameObject(factory, id, name, tag, transform) {
    }

    void Awake() override;
    void Update(float dt) override;
    void Uninit() override;

private:
    SpotLightComponent* m_spot = nullptr;
    SearchLightControllerComponent* m_ctrl = nullptr;
};