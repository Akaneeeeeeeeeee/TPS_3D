#include "ObjectContactListener.h"
#include "Framework/PhysicsSystem/PhysicsLayer.h"
#include <Jolt/Physics/Collision/ContactListener.h>

//void GameContactListener::OnContactAdded(
//    const JPH::Body& body1, const JPH::Body& body2,
//    const JPH::ContactManifold& manifold,
//    JPH::ContactSettings& settings
//)
//{
//    // レイヤー取得
//    auto layer1 = body1.GetObjectLayer();
//    auto layer2 = body2.GetObjectLayer();
//
//    // 「Player(or 落下物) × Terrain 」だけを対象にする
//    if (!IsSoundTargetPair(layer1, layer2))
//        return;
//
//    // 接触位置（複数あるので代表を取る）
//    if (manifold.mRelativeContactPointsOn1.size() == 0)
//        return;
//
//    JPH::Vec3 localPoint = manifold.mRelativeContactPointsOn1[0];
//    JPH::RVec3 worldPoint = manifold.mBaseOffset + localPoint;
//
//    // 音イベントを発行（ゲーム側のシステムへ）
//    SoundEvent ev{};
//    ev.position = ConvertToGameVector(worldPoint);
//    ev.type = SoundType::Impact;
//    ev.power = CalcImpactPower(body1, body2, manifold); // 速度差などから(なくても可)
//
//    SoundManager::Get().PostWorldSound(ev);
//}
