#pragma once
#include "system/Framework/GameObject/Character/Character.h"

// 前方宣言
class CharacterVirtualComponent;
class CameraComponent;
class ThrowComponent;

/*
* @brief	プレイヤークラス
* @detail	ゲーム内のプレイヤーキャラクターを表すクラス
* @remark	Characterクラスを継承し、初期化、更新、描画、終了処理のメソッドをオーバーライドする
* @auther	赤根和樹
* @date		2025/10/11
* 
* todo : 入力はコマンドパターンでやりたいよねー
*/
class Player final : public Character
{
public:
	Player() = default;
	Player(ComponentFactory* factory, const uint64_t id,
		const std::string& name = "", const Tag& tag = Tag::Player,
		const Transform& transform = Transform::One());
	~Player();

	void Awake(void) override;
	void Start(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	JPH::BodyID GetInnerBodyID(void) const;

	CameraComponent* GetCamera(void) { return m_pCamera; }

	void GetVisibilitySamplePoints(const Vector3& eyePos, std::vector<Vector3>& out) const;	// 視線判定用のサンプリング点を取得

private:
	struct InputState
	{
		Vector3 inputDir = Vector3::Zero;
		float amount = 0.0f;

		bool aiming = false;
		bool recenter = false;
		bool wantsJump = false;
		bool isCrouching = false;

		bool throwPressed = false; // 押した瞬間
	};

private:
	// 入力→状態
	InputState ReadInputState(float dt);

	// 移動
	void BuildMoveDirection(const InputState& in, Vector3& outMoveDir, float& outMoveAmount);
	void ApplyFacingRotation(const InputState& in, const Vector3& moveDir, float moveAmount);
	void ApplyStance(const InputState& in);
	void ApplyMoveToCharacterVirtual(const Vector3& moveDir, float moveAmount, bool wantsJump);

	// アニメ（通常時のみ。構え中は ThrowComponent が主導）
	void UpdateMovementAnimation(const InputState& in);

	// 足音
	void UpdateFootstep(float dt);

	// カメラ
	void UpdateCamera(float dt, const InputState& in);

	// Throw（通知と PostUpdate 呼び出し）
	void UpdateThrowNotify(const InputState& in);

private:
	CharacterVirtualComponent* m_pCharaVirtualComp = nullptr;
	CameraComponent* m_pCamera = nullptr;
	ThrowComponent* m_pThrowComp = nullptr;

	// 足音用
	static constexpr float FOOTSTEP_BASE_INTERVAL = 0.3f;
	static constexpr float FOOTSTEP_BASE_RADIUS = 800.0f;
	static constexpr float FOOTSTEP_BASE_LOUDNESS = 1.0f;

	float m_FootstepTimer = 0.0f;
	float m_FootstepIntervalRun = 0.30f;  // 走り時の間隔
	float m_FootstepIntervalCrouch = 0.50f;  // しゃがみ歩き
	bool  m_FootstepEnabled = true;   // 必要なら ON/OFF できるように

	// カメラ補間用（static をやめて Player が持つ）
	float m_CamAzimuth = 0.0f;
	float m_CamElevation = 0.0f;

	float m_CamRadiusCur = 800.0f;
	float m_CamShoulderCur = 0.0f;
	float m_CamLookAtHeightCur = 100.0f;
	float m_CamNearCur = 1.0f;

	// 前フレームで接地していたか
	bool  m_WasOnGround = false;

	// 前フレームの構え状態
	bool  m_PrevAiming = false;
	float m_PrevRightTrigger = 0.0f; // 右トリガーの押し込み判定用（GetRightTriggerがfloat前提）
};