#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "PhysicsLayer.h"
#include <Jolt/Core/JobSystemThreadPool.h>
#include <memory>
#include <vector>
#include "JoltDebugRendererDX11.h"

class PhysicsComponent; // å‰æ–¹å®£è¨€
#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
class JoltDebugRendererDX11;
#endif

/*
* @brief    •¨—‰‰Zƒ}ƒl[ƒWƒƒ[
* @detail   Jolt Physics‚Ì•¨—‰‰ZƒVƒXƒeƒ€‚ğŠÇ—‚·‚éƒNƒ‰ƒX
* @remark   PhysicsComponent‚Ì”h¶ƒNƒ‰ƒX(Rigidbody,ColliderŒn)‚ª“o˜^‚³‚ê‚Ä‚¢‚é
* @remark   •¨—ƒIƒuƒWƒFƒNƒg‚ğŠÇ—‚µA•¨—‰‰Z‚Ì‰Šú‰»‚ÆXV‚ğs‚¤
*/
class PhysicsManager
{
public:
    ~PhysicsManager();
    void Init(void);
    void Update(const float deltaTime);

#ifdef _DEBUG
    void DebugDraw(void);
#endif
    //void DebugDraw(const DirectX::XMMATRIX& vp);

    void Register(PhysicsComponent* rb);
    void UnRegister(PhysicsComponent* rb);

    JPH::PhysicsSystem& GetSystem() { return m_System; }
    JPH::BodyInterface& GetBodyInterface() { return m_System.GetBodyInterface(); }

    // Jolt Physics‚ÌTempAllocator‚ğ•Ô‚·ƒƒ\ƒbƒh‚ğ’Ç‰Á
    JPH::TempAllocator* GetTempAllocator()
    {
        return m_TempAllocator.get();
    }

private:
    JPH::PhysicsSystem m_System;
    std::unique_ptr<JPH::TempAllocatorImpl> m_TempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_JobSystem;
    std::vector<PhysicsComponent*> m_PhysicsObjects;
#if defined(JPH_DEBUG_RENDERER) && defined(_DEBUG)
    std::unique_ptr<JoltDebugRendererDX11> m_DebugRenderer;
#endif
};