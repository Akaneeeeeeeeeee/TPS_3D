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
class GameObject;

#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
class JoltDebugRendererDX11;
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
#endif

    void Register(PhysicsComponent* rb);
    void UnRegister(PhysicsComponent* rb);

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

private:
    JPH::PhysicsSystem m_System;
    std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
    std::vector<PhysicsComponent*> m_PhysicsObjects;
    ObjectContactListener m_ObjectContactListener;
    CharacterContactListenerImpl m_CharacterContactListener;

#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
    std::unique_ptr<JoltDebugRendererDX11> m_DebugRenderer;
#endif
};