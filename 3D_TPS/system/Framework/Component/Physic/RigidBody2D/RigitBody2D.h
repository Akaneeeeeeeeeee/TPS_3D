#pragma once
#include "../../IComponent/IComponent.h"

using namespace SimpleMath;

/**
 * @brief 力の加え方
 * 
 * 参考サイト
 * https://shibuya24.info/entry/unity-rigidbody-addforce#ForceMode%E3%80%8CForce%EF%BC%88%E3%83%95%E3%82%A9%E3%83%BC%E3%82%B9%EF%BC%89%E3%80%8D
 * https://rightcode.co.jp/blogs/25168
 * 
 * accelerationとvelocitychangeは2Dには存在しない
 * が、velocitychangeは処理的には速度を直接書き換えてるだけなのでここでは実装してしまう
*/
enum class ForceMode2D {
    Force,              // 質量を考慮して継続的な力を加える(デフォルトはこれ)
    // Acceleration,       // 質量を"無視"して加速度を加える
    Impulse,            // 質量を考慮して瞬間的な力を加える
    VelocityChange      // 質量を"無視"して速度を直接変更する
};

/**
 * @brief 物理演算
 * 
 * このコンポーネントを取りつけていても当たり判定を検知しない設定に変更できるようにしたい
 * 
 * 重力、摩擦などが必要
 * 
 * 
 * 2. 最終的なコードのイメージ
C++ で RigidBody::Update(float deltaTime) を実装するなら、次のような形になります。

cpp
コピーする
編集する
void RigidBody::Update(float deltaTime) {
    // 1. 力の適用（例: 重力）
    force += mass * gravity;

    // 2. 加速度を求める
    Vector3 acceleration = force / mass;

    // 3. 速度を更新
    velocity += acceleration * deltaTime;

    // 4. 位置を更新
    position += velocity * deltaTime;

    // 5. 角速度の更新
    Vector3 angularAcceleration = torque / inertia;
    angularVelocity += angularAcceleration * deltaTime;
    rotation += angularVelocity * deltaTime;

    // 6. 衝突判定
    if (isColliding) {
        velocity = Reflect(velocity, collisionNormal) * restitution;
    }

    // 7. 力をリセット（毎フレームリセットして新しい力を受け付ける）
    force = Vector3(0, 0, 0);
    torque = Vector3(0, 0, 0);
}
3. まとめ
update の流れは次のようになります：

外力（重力・操作）を加える
加速度を求めて速度を更新
速度を使って座標を更新
角運動（回転）も計算
衝突処理
毎フレームの力をリセット
+α: 高精度な物理を目指すなら
オイラー積分の代わりにVerlet積分やRunge-Kutta法を使う
衝突検出をより正確に（AABB, OBB, GJKアルゴリズム）
剛体同士の摩擦や弾性衝突の計算
固定時間ステップの導入（物理更新は固定フレームで実施）
*/
class RigidBody2D :public IComponent
{
public:
    RigidBody2D(Object* _owner) :IComponent(_owner)
    {
        m_Velocity = { 0.0f };
        m_Direction = { 0.0f };
        m_Force = { 0.0f };
        m_Mass = 1.0f;
        m_GravityScale = 0.98f;
        DetectCollision = true;
        UseGravity = true;
        IsKinematic = false;
    };
    ~RigidBody2D() {};


    //--------------------------------
    //        オーバーライド関数
    //--------------------------------
    void Init(void) override;
    void Update(void) override;
    void Uninit(void) override;


    //--------------------------------
    //          オリジナル関数
    //--------------------------------
    
    // 力を加える
    void AddForce(Vector3 _force, ForceMode2D _mode);
	
private:
    // 速度(これは毎フレーム変化する値)
    Vector3 m_Velocity;

    // 移動用方向ベクトル(Transform.Rotaionは回転を扱うものなので別物)
    Vector3 m_Direction;

    // 力
    Vector3 m_Force;

    // 質量(単位：kg)
    float m_Mass;

    // 重力
    float m_GravityScale;
    
    // 衝突判定を有効にするか(デフォルトではON)
	bool DetectCollision;

    // 重力の影響を受けるか(デフォルトではON)
    bool UseGravity;

    // 物理挙動の有無(デフォルトはOFF(物理挙動させる))
    bool IsKinematic;

    // 回転の影響を受けるか
    //bool FreezeRotation;
};


