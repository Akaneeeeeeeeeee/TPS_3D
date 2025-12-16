#pragma once
#include "Framework/GameObject/Character/Character.h"

// 前方宣言
class CharacterVirtualComponent;

/*
* @brief	タイトル用プレイヤークラス
* @detail	タイトルシーン内のプレイヤー(決まった動きだけを行うため、プレイアブルキャラクターとは別で定義)
* @auther	赤根　和樹
* @date		2025/12/16
*/
class TitlePlayerActor : public Character
{
public:
    TitlePlayerActor() = delete;
    TitlePlayerActor(ComponentFactory* factory, const uint64_t id,
        const std::string& name = "TitlePlayer",
        const Tag& tag = Tag::Player,
        const Transform& transform = Transform::One())
        : Character(factory, id, name, tag, transform)
    {
    }

    ~TitlePlayerActor() override = default;

    void Awake(void) override;
    void Start(void) override;           // あなたの基底にあるなら
    void Update(const float dt) override;
    void Draw(void) const override;
    void Uninit(void) override;

    // ---- タイトル台本から渡す操作 ----
    // 位置・回転を外から決め打ちしたい場合
    void SetPose(const Vector3& pos, const Quaternion& rot)
    {
        m_TargetPos = pos;
        m_TargetRot = rot;
        m_UseTargetPose = true;
    }

    // 速度でスーッと動かしたい場合（台本側がdir/amountを決める）
    void SetMoveInput(const Vector3& dir, float amount)
    {
        m_MoveDir = dir;
        m_MoveAmount = std::clamp(amount, 0.0f, 1.0f);
    }

    // 走り/歩き/待機だけで良い（必要なら増やす）
    enum class TitleAnim
    {
        Idle,
        Walk,
        Run,
    };

    void SetAnim(TitleAnim a) { m_TargetAnim = a; }

    // 台本用：向きを進行方向へ向けたい/固定したい
    void SetFaceMoveDir(bool v) { m_FaceMoveDir = v; }

private:
    // ---- 台本入力 ----
    Vector3   m_MoveDir = Vector3::Zero;  // 正規化済みが理想（z前進などはあなたの座標系に合わせる）
    float     m_MoveAmount = 0.0f;

    bool      m_UseTargetPose = false;
    Vector3   m_TargetPos = Vector3::Zero;
    Quaternion m_TargetRot = Quaternion::Identity;

    CharacterVirtualComponent* m_pCharaVirtualComp = nullptr;

    bool    m_FaceMoveDir = true;
    bool    m_IsCrouching = false;

    // ---- アニメ状態 ----
    TitleAnim m_TargetAnim = TitleAnim::Idle;

    // クリップ（AssetManagerから取って保持）
    // 型はあなたの実装に合わせて（例：CAnimationData / aiAnimation 等）
    aiAnimation* m_Idle = nullptr;
    aiAnimation* m_CrouchWalk = nullptr;
    aiAnimation* m_Walk = nullptr;
    aiAnimation* m_Run = nullptr;

	CameraComponent* m_pCamera = nullptr;

private:
    void ApplyAnimation(float dt);
    void ApplyMovement(float dt);
};