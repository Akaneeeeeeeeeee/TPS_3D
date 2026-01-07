#pragma once
#include "Framework/PhysicsSystem/Physics.h"
#include <Jolt/Physics/Collision/ContactListener.h>

// エイリアス
using JPH::ValidateResult;
using JPH::Body;
using JPH::RVec3Arg;
using JPH::CollideShapeResult;
using JPH::ContactManifold;
using JPH::ContactSettings;
using JPH::SubShapeIDPair;

// 前方宣言
class PhysicsManager;

/*
* @brief	オブジェクト接触リスナー
* @detail   Jolt Physicsの接触イベントをり、イベントを発行するクラス
* @remark   各イベントは、ContactListenerの仮想関数をオーバーライドして実装する
* @remark   物理演算中に呼び出されるため、スレッドセーフな実装を行うわないとだめ
* @auther	赤根 和樹
* @date     2025/12/11
*/
class ObjectContactListener : public JPH::ContactListener
{
public:
    explicit ObjectContactListener(PhysicsManager& owner);
	~ObjectContactListener() override = default;

    // 1. そもそもこの接触を許可するかどうか
    ValidateResult OnContactValidate(
        const Body& body1,
        const Body& body2,
        RVec3Arg             baseOffset,
        const CollideShapeResult& result
    ) override;

    // 2. 新しく接触が発生したとき（Enter）
    void OnContactAdded(
        const Body& body1,
        const Body& body2,
        const ContactManifold& manifold,
        ContactSettings& settings
    ) override;

    // 3. すでに接触していたペアが、同じように接触し続けているとき（Stay）
    void OnContactPersisted(
        const Body& body1,
        const Body& body2,
        const ContactManifold& manifold,
        ContactSettings& settings
    ) override;

    // 4. 接触が終わったとき（Exit）
    void OnContactRemoved(
        const SubShapeIDPair& subShapePair
    ) override;

private:
    PhysicsManager& m_Owner;

    // Removed時に Body が取れないケースがあるので、BodyIDペア→情報を保存しておく
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
        bool isTrigger = false; // sensor1 && !sensor2 か !sensor1 && sensor2 のとき true
        uint64_t id1 = 0;
        uint64_t id2 = 0;
        int refCount = 0;       // サブシェイプ複数接触の重複に備える
    };

    static PairKey MakeKey(const JPH::BodyID& id1, const JPH::BodyID& id2)
    {
        uint32_t a = id1.GetIndexAndSequenceNumber();
        uint32_t b = id2.GetIndexAndSequenceNumber();
        if (a < b) return PairKey{ a, b };
        else       return PairKey{ b, a };
    }

    std::mutex m_PairMtx;
    std::unordered_map<PairKey, PairInfo, PairKeyHash> m_Pairs;
};

