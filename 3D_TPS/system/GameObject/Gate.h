#pragma once
#include "Framework/GameObject/GameObject.h"

class CStaticMesh;
class StaticMeshRendererComponent;
class StaticMeshCollider;
class Rigidbody;
class BoxCollider;
class SoundEmitterComponent;

class Gate final : public GameObject
{
public:
    Gate(ComponentFactory* factory, uint64_t id,
        const std::string& name = "", const Tag& tag = Tag::Object,
        const Transform& transform = Transform::One())
        : GameObject(factory, id, name, tag, transform) {
    }

    void Awake() override;
    void Start() override;
    void Update(float dt) override;

    void Toggle();
    void SetOpen(bool open);
    bool IsOpen() const { return m_IsOpen; }
	bool IsMoving() const { return m_IsMoving; }

    // ここは「方向」だけ渡す
    void SetHingeLocalOffset(const Vector3& dir);

    void SetOpenYawDeg(float deg) { m_OpenYawDeg = deg; }
    void SetSpeedDegPerSec(float deg) { m_SpeedDegPerSec = deg; }

private:
    void BuildDoorChild(void);
    void ApplyPivotYaw(float yawDeg);
    void StartMoveLoopSound(void);
    void StopMoveLoopSound(void);
private:
	SoundEmitterComponent* m_SoundEmitter = nullptr;

    GameObject* m_Door = nullptr;     // 子（ドア本体）

    Quaternion m_BaseRot = Quaternion::Identity; // Gate設置時の回転（任意）
    Vector3    m_DoorScale = Vector3::One;       // 元のscaleは子へ移す

    // 「蝶番→ドア中心」の方向（正規化して使う）
    Vector3 m_HingeDirLocal = Vector3(1, 0, 0);

    float m_CurYawDeg = 0.0f;
    float m_TargetYawDeg = 0.0f;
    float m_OpenYawDeg = 90.0f;
    float m_SpeedDegPerSec = 120.0f;
    bool  m_IsOpen = false;

	// 音関連
    bool  m_IsMoving = false;
    float m_MoveSoundTimer = 0.0f;
    float m_MoveSoundInterval = 0.2f;   // 0.1〜0.3くらいで調整
    float m_MoveSoundRadius = 1200.0f;
    float m_MoveSoundVolume = 1.0f;
    float m_MoveSoundLoudness = 1.0f;

	bool m_LoopPlaying = false; // 音鳴らしてるか
    bool m_SentStartHear = false; // 動き始めの1回だけEmitするため
};