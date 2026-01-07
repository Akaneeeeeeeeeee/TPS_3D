#pragma once
#include "Framework/PhysicsSystem/Physics.h"
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/Body.h>

class PhysicsManager;

class CharacterContactListenerImpl final : public JPH::CharacterContactListener
{
public:
    explicit CharacterContactListenerImpl(PhysicsManager& owner);
    ~CharacterContactListenerImpl() override = default;

    // そもそもこの接触を許可するか
    bool OnContactValidate(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        const JPH::SubShapeID& inSubShapeID2
    ) override;

#if (JPH_VERSION_MAJOR > 5) || (JPH_VERSION_MAJOR == 5 && JPH_VERSION_MINOR >= 3)
    // 新規接触（Enter候補）
    void OnContactAdded(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        const JPH::SubShapeID& inSubShapeID2,
        JPH::RVec3Arg inContactPosition,
        JPH::Vec3Arg  inContactNormal,
        JPH::CharacterContactSettings& ioSettings
    ) override;

    // 接触継続（今回は未使用：Stayを作りたくなったらここ）
    void OnContactPersisted(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        const JPH::SubShapeID& inSubShapeID2,
        JPH::RVec3Arg inContactPosition,
        JPH::Vec3Arg  inContactNormal,
        JPH::CharacterContactSettings& ioSettings
    ) override;

    // 接触終了（Exit候補）
    void OnContactRemoved(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        const JPH::SubShapeID& inSubShapeID2
    ) override;
#else
    // v5.1系など（Removedが無い）：Enter重複抑制だけやる
    void OnContactAdded(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        const JPH::SubShapeID& inSubShapeID2,
        JPH::RVec3Arg inContactPosition,
        JPH::Vec3Arg  inContactNormal,
        JPH::CharacterContactSettings& ioSettings
    ) override;
#endif

private:
    PhysicsManager& m_Owner;

    // BodyIDペアで追跡（RemovedでBodyを触らないため）
    struct PairKey
    {
        uint32_t a;
        uint32_t b;
        bool operator==(const PairKey& r) const { return a == r.a && b == r.b; }
    };
    struct PairKeyHash
    {
        size_t operator()(const PairKey& k) const noexcept
        {
            return (size_t(k.a) << 1) ^ size_t(k.b);
        }
    };

    struct PairInfo
    {
        bool isTrigger = false; // 相手がsensorならtrue（＝TriggerEnter/Exitにする）
        uint64_t charId = 0;
        uint64_t otherId = 0;
        int refCount = 0;       // サブシェイプ複数/重複に備える
    };

    static PairKey MakeKey(const JPH::BodyID& id1, const JPH::BodyID& id2)
    {
        uint32_t a = id1.GetIndexAndSequenceNumber();
        uint32_t b = id2.GetIndexAndSequenceNumber();
        if (a < b) return PairKey{ a, b };
        else       return PairKey{ b, a };
    }

    bool ReadUserDataAndFlags(
        const JPH::CharacterVirtual* inCharacter,
        const JPH::BodyID& inBodyID2,
        uint64_t& outCharId,
        uint64_t& outOtherId,
        bool& outIsTrigger,
        JPH::BodyID& outInnerBodyID // key用（Removedでも使う）
    );

    std::mutex m_PairMtx;
    std::unordered_map<PairKey, PairInfo, PairKeyHash> m_Pairs;
};
