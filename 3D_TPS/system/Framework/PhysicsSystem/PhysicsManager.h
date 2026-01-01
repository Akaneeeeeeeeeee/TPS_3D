#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <memory>
#include <vector>
#include "PhysicsLayer.h"
#include "JoltDebugRendererDX11.h"
#include "Framework/PhysicsSystem/ObjectContactListener.h"
#include "Framework/PhysicsSystem/CharacterContactListener.h"


// 前方宣言
class PhysicsComponent;
class ObjectManager;
class GameObject;

#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
class JoltDebugRendererDX11;
class CameraManager;
#endif

/*
* @brief    物理演算マネージャー
* @detail   Jolt Physicsの物理演算システムを管理するクラス
* @remark   PhysicsComponentの派生クラス(Rigidbody,Collider系)が登録されている
* @remark   物理オブジェクトを管理し、物理演算の初期化と更新を行う
* @auther   赤根 和樹
* @date     2025/11/XX(後で確認)
*/
class PhysicsManager
{
public:
	PhysicsManager();
    ~PhysicsManager();
    void Init(void);
    void Update(const float deltaTime);

#ifdef _DEBUG
    void DebugDraw(void);
	void SetCameraManager(class CameraManager* cameraManager);
#endif

    void Register(PhysicsComponent* rb);
    void UnRegister(PhysicsComponent* rb);

    void SetObjectManager(ObjectManager* om) { m_ObjectManager = om; }

    // メインスレッドで呼ぶ（Physics.Updateの直後）
    void DispatchCollisionEvents(void);

    JPH::PhysicsSystem& GetSystem(void) { return m_System; }
    JPH::BodyInterface& GetBodyInterface(void) { return m_System.GetBodyInterface(); }

    // Jolt PhysicsのTempAllocatorを返すメソッドを追加
    JPH::TempAllocator* GetTempAllocator(void)
    {
        return m_TempAllocator.get();
    }
    // CharacterVirtual に渡すためのリスナー取得関数
    JPH::CharacterContactListener* GetCharacterContactListener(void)
    {
        return &m_CharacterContactListener;
    }

	// 当たり判定イベント
    void OnCollisionEnter(GameObject& a, GameObject& b);
    void OnCollisionStay(GameObject& a, GameObject& b);
    void OnCollisionExit(GameObject& a, GameObject& b);

    void OnTriggerEnter(GameObject& trigger, GameObject& other);
    void OnTriggerStay(GameObject& trigger, GameObject& other);
    void OnTriggerExit(GameObject& trigger, GameObject& other);

    // キャラ用イベント(衝突時だけ)
	void OnCharacterCollisionEnter(GameObject& a, GameObject& b);

    // ignoreBody は不要なら省略できるようにしておく
    bool RaycastClosest(
        const Vector3& from, const Vector3& to,
        JPH::RayCastResult& outHit,
        const JPH::BodyID& ignoreBody = JPH::BodyID()) const;

    bool IsOccluded(
        const Vector3& from, const Vector3& to,
        const JPH::BodyID& ignoreBody = JPH::BodyID()) const;


    struct CollisionEvent
    {
        enum class Type
        {
            CollisionEnter,
            CollisionExit,
            TriggerEnter,
            TriggerExit,
            CharacterEnter,
        };

        Type type;
        uint64_t aId;
        uint64_t bId;
    };

    // 物理スレッドからは「積むだけ」
    void EnqueueEvent(CollisionEvent::Type type, uint64_t aId, uint64_t bId);

private:
    JPH::PhysicsSystem m_System;
    std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
    std::vector<PhysicsComponent*> m_PhysicsObjects;
    ObjectContactListener m_ObjectContactListener;
    CharacterContactListenerImpl m_CharacterContactListener;

    ObjectManager* m_ObjectManager = nullptr;

    std::mutex m_EventMtx;
    std::vector<CollisionEvent> m_EventQueue;

#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
    std::unique_ptr<JoltDebugRendererDX11> m_DebugRenderer;
	CameraManager* m_CameraManager = nullptr;
#endif
};