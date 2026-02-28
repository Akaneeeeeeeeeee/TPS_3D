#pragma once
#include "Framework/GameObject/GameObject.h"

class Rigidbody;
class BoxCollider;
class StaticMeshRendererComponent;
class Gate;
class SoundEmitterComponent;

class GateButton final : public GameObject
{
public:
    GateButton(ComponentFactory* factory, uint64_t id,
        const std::string& name = "", const Tag& tag = Tag::Object,
        const Transform& transform = Transform::One())
        : GameObject(factory, id, name, tag, transform) {
    }

    void Awake() override;
    void Start() override;
    void Update(float dt) override;
    void Uninit() override {}

    void BindDoor(Gate* door) { m_TargetDoor = door; }

	void OnCollisionCharacterEnter(GameObject& other) override;
	void OnCollisionCharacterExit(GameObject& other) override;

private:
    bool CanUseByPlayer() const;

private:
    // 見た目
    StaticMeshRendererComponent* m_Render = nullptr;
	SoundEmitterComponent* m_SoundEmitter = nullptr;

    // 近接判定（トリガーにする）
    BoxCollider* m_TriggerBox = nullptr;
    Rigidbody* m_RB = nullptr;
    GameObject* m_Model = nullptr;
    Vector3 m_ModelScale = Vector3::One;

    // 対象ドア
    Gate* m_TargetDoor = nullptr;

    // 設定
    float m_UseRadius = 200.0f;   // 距離で押す方式にするならこれ
    float m_CooldownSec = 0.2f;

    // 状態
    float m_Cooldown = 0.0f;
    bool  m_SyncedOpen = false;   // ドアの状態を保持（同期表示用）

    bool m_PlayerInRange = false;
};
