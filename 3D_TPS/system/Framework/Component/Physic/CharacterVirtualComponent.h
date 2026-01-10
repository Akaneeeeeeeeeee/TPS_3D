#pragma once
#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"
#include <Jolt/Physics/Character/CharacterVirtual.h>

// 前方宣言
class PhysicsManager;

/*
* @brief	バーチャルキャラクターコンポーネント
* @detail	キャラクターの物理演算を提供するコンポーネント
* @remark   当たり判定のため、カプセルの形状を持つ
* @remark   他の物理系コンポーネントと違い、JPH::CharacterVirtualを使用してキャラクターの移動を制御する
* @remark   主にプレイヤーキャラクターやNPCの移動に使用する（Joltのリファレンス参照）
* @auther	赤根 和樹
* @date     2025/11/16
*/
class CharacterVirtualComponent : public IComponent
{
public:
	DECLARE_COMPONENT_TYPE(CharacterVirtualComponent, IComponent)
    CharacterVirtualComponent()
        : m_Physics(nullptr)
        , m_Character(nullptr)
        , m_InnerBodyID(JPH::BodyID())   // 無効 ID
    {
    }
    ~CharacterVirtualComponent() noexcept override; // デストラクタの例外指定を親クラスと一致させるため、noexcept

	// 姿勢
    enum class Stance
    {
		Stand,  // 立ち
		Crouch, // しゃがみ
		Prone,  // 伏せ
    };

	void Init(void) override;
	void Update(const float deltaTime) override;
	void Uninit(void) override;

	void Attach(EngineServices& context) override;
	void Detach(void) override;
   
	void SetStance(Stance stance);

    // --- 入力 API ---
    void SetMoveDir(const Vector3& dir) { m_MoveDir = dir; }
    void RequestJump(void) { m_WantsJump = true; }
	void SetOffset(const Vector3& offset) { m_Offset = offset; }

	Vector3 GetLinearVelocity(void) const;
    float GetHorizontalSpeed(void) const;
    float GetCurrentHalfHeight(void) const;
	float GetRadius(void) const { return m_Radius; }
	Stance GetStance(void) const { return m_Stance; }
    float GetMoveSpeedCoeff() const;
    float GetFootstepIntervalCoeff() const;
    float GetFootstepRadiusCoeff() const;
    float GetFootstepLoudnessCoeff() const;

	// カプセル形状
	void SetCapsule(float halfHeight, float radius)
	{
        m_HalfHeight = halfHeight;
		m_Radius = radius;
	}
    
	// Inner Body ID 取得
    const JPH::BodyID& GetInnerBodyID(void) const { return m_InnerBodyID; }
    
    const JPH::Shape* GetCurrentShape(void) const;
	// 地面接地判定
    bool IsOnGround(void) const;
    // 完全停止用
    void Stop(void);
	// テレポート(スタック解消用)
	void Teleport(const Vector3& worldPos);
    void SetMoveInput(const Vector3& dir, float amount)
    {
        // 入力方向（水平のみ使う：坂や段差のYで遅く見える/ガタつくのを避ける）
        m_MoveDir = dir;
        m_MoveDir.y = 0.0f;

        // 倒し具合（0..1）
        m_MoveAmount = std::clamp(amount, 0.0f, 1.0f);

        // 方向がほぼ無いなら量も0扱い（ゼロ割/不安定防止）
        if (m_MoveDir.LengthSquared() < 1e-6f)
        {
            m_MoveAmount = 0.0f;
            m_MoveDir = Vector3::Zero;
        }
    }

    void SetMoveSpeedScale(float s) { m_MoveSpeedScale = std::max(0.0f, s); }

private:
    // 姿勢ごとの係数
    struct StanceCoeff
    {
		float moveSpeed;        // 移動速度係数
		float footstepInterval; // 足音間隔
		float footstepRadius;   // 足音範囲
		float footstepLoudness; // 足音大きさ
    };

    static constexpr StanceCoeff StandCoeff{ 1.0f, 1.0f, 1.0f, 1.0f };
    static constexpr StanceCoeff CrouchCoeff{ 0.5f, 1.6f, 0.5f, 0.4f };
    static constexpr StanceCoeff ProneCoeff{ 0.25f, 2.0f, 0.3f, 0.2f };

    // 内部ヘルパ
    void BuildStanceShapes();
    PhysicsManager* m_Physics = nullptr;
    JPH::CharacterVirtual* m_Character = nullptr;
	JPH::BodyID m_InnerBodyID;      // Inner BodyのID

    // 姿勢ごとの Shape を保持
    JPH::RefConst<JPH::Shape> m_StandShape;
    JPH::RefConst<JPH::Shape> m_CrouchShape;
    JPH::RefConst<JPH::Shape> m_ProneShape;

	// 現在の姿勢
	Stance m_Stance = Stance::Stand;

	// カプセル形状パラメータ
    float   m_HalfHeight = 60.0f;
	Vector3 m_Offset = Vector3::Zero;

    // 姿勢ごとのパラメータ
    float   m_StandHalfHeight = 60.0f;
    float   m_CrouchHalfHeight = 20.0f;
    float   m_ProneHalfHeight = 20.0f;
    float   m_Radius = 35.0f;

	// 姿勢ごとのオフセット
    Vector3 m_StandOffset{};
    Vector3 m_CrouchOffset{};
    Vector3 m_ProneOffset{};

	// 移動制御用パラメータ
    Vector3 m_MoveDir = Vector3::Zero;
    bool    m_WantsJump = false;
	float   m_MoveAmount = 0.0f; // 0.0 ～ 1.0

    // チューニング用
    float m_BaseMoveSpeed = 500.0f; // 立ち状態を基準にした最高速度
    float m_MoveSpeed = 500.0f;     // 最高速度
    float m_JumpSpeed = 400.0f;
    float m_MoveSpeedScale = 1.0f; // 1.0=基準、0.5=半分など
};
