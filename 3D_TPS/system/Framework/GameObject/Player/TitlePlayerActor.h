#pragma once
#include "Framework/GameObject/Character/Character.h"

// 前方宣言
class CharacterVirtualComponent;
class Terrain;

/*
* @brief	タイトル用プレイヤークラス
* @detail	タイトルシーン内のプレイヤー(決まった動きだけを行うため、プレイアブルキャラクターとは別で定義)
* @auther	赤根　和樹
* @date		2025/12/16
*/
class TitlePlayerActor : public Character
{
public:
    DECLARE_GAMEOBJECT_TYPE(TitlePlayerActor, Character)
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
        CheckOverWall,
        ThrowStone,
    };

    enum class FaceMode
    {
        FaceMoveDir,     // 移動方向に向く（従来）
        FaceManualDir,   // m_FaceDir を使う
        FaceTargetPos    // m_FaceTargetPos を見る
    };

    void SetAnim(TitleAnim a) { m_TargetAnim = a; }

    // 台本用：向きを進行方向へ向けたい/固定したい
    //void SetFaceMoveDir(bool v) { m_FaceMoveDir = v; }

	void SetTerrain(Terrain* terrain) { m_Terrain = terrain; }

    // TitlePlayerActor.h
    void SetMove(const Vector3& dir, float amount) { m_MoveDir = dir; m_MoveAmount = amount; }
    void SetFaceMode(FaceMode m) { m_FaceMode = m; }
    void SetFaceDir(const Vector3& dir) { m_FaceDir = dir; }
    void SetFaceTargetPos(const Vector3& p) { m_FaceTargetPos = p; }
    void SetTargetAnim(TitleAnim a) { m_TargetAnim = a; }
    void SetSidewaysRight(bool v) { m_SidewaysRight = v; }


private:
    // ---- 台本入力 ----
    Vector3   m_MoveDir = Vector3::Zero;  // 正規化済みが理想（z前進などはあなたの座標系に合わせる）
    float     m_MoveAmount = 0.0f;

    bool      m_UseTargetPose = false;
    Vector3   m_TargetPos = Vector3::Zero;
    Quaternion m_TargetRot = Quaternion::Identity;

    CharacterVirtualComponent* m_pCharaVirtualComp = nullptr;
    //Vector3  m_MoveDir = Vector3(0, 0, 1);   // 移動方向（ワールド）
    //float    m_MoveAmount = 0.0f;             // 0..1

    FaceMode m_FaceMode = FaceMode::FaceMoveDir;
    Vector3  m_FaceDir = Vector3(0, 0, 1);   // 体の向き（ワールド）
    Vector3  m_FaceTargetPos = Vector3::Zero; // 見る位置（ワールド）

    Vector3  m_LastMoveDir = Vector3(0, 0, 1);   // 停止中に向きが壊れないよう保持

    bool    m_IsCrouching = false;
	bool    m_SidewaysRight = false;

    // ---- アニメ状態 ----
    TitleAnim m_TargetAnim = TitleAnim::Idle;

    // クリップ（AssetManagerから取って保持）
    // 型はあなたの実装に合わせて（例：CAnimationData / aiAnimation 等）
    aiAnimation* m_Idle = nullptr;
    aiAnimation* m_CrouchWalk = nullptr;
    aiAnimation* m_CheckOverWall = nullptr;
    aiAnimation* m_Run = nullptr;
    aiAnimation* m_CoveredCrouchWalk = nullptr;
    aiAnimation* m_ThrowStone = nullptr;

	CameraComponent* m_pCamera = nullptr;
    Terrain* m_Terrain = nullptr;

private:
    void ApplyAnimation(float dt);
    void ApplyMovement(float dt);
    void SetupFixedTitleCamera(void);
};
