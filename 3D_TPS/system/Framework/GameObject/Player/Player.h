#pragma once
#include "system/Framework/GameObject/Character/Character.h"
#include "system/camera.h"

// 前方宣言
class CharacterVirtualComponent;

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

	void Init(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	JPH::BodyID GetInnerBodyID(void) const;

	void SetCamera(FreeCamera* cam) { m_pCamera = cam; }
	void GetVisibilitySamplePoints(const Vector3& eyePos, std::vector<Vector3>& out) const;	// 視線判定用のサンプリング点を取得
private:
	FreeCamera* m_pCamera = nullptr;
	CharacterVirtualComponent* m_pCharaVirtualComp = nullptr;

	// 足音用
	static constexpr float FOOTSTEP_BASE_INTERVAL = 0.3f;
	static constexpr float FOOTSTEP_BASE_RADIUS = 800.0f;
	static constexpr float FOOTSTEP_BASE_LOUDNESS = 1.0f;

	float m_FootstepTimer = 0.0f;
	float m_FootstepIntervalRun = 0.30f;  // 走り時の間隔
	float m_FootstepIntervalCrouch = 0.50f;  // しゃがみ歩き
	bool  m_FootstepEnabled = true;   // 必要なら ON/OFF できるように

	// 前フレームで接地していたか
	bool  m_WasOnGround = false;
};