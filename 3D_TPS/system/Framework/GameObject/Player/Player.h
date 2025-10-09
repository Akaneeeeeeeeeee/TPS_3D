#pragma once
#include "system/Framework/GameObject/Character/Character.h"
#include "system/camera.h"

/// <summary>
/// ゲーム内のプレイヤーキャラクターを表すクラスです。
/// Character クラスを継承し、初期化、更新、描画、終了処理のメソッドをオーバーライドします。
/// 
/// TODO:シーン終了時にカメラの座標が原点付近になるバグを修正
/// 
/// </summary>
class Player final : public Character
{
public:
	Player() = default;
	Player(EngineContext& context, uint64_t id, const std::string& name = "", const Tag& tag = Tag::Player);
	~Player();

	void Init(void) override;
	void Update(uint64_t deltatime) override;
	void Draw(uint64_t deltatime) override;
	void Uninit(void) override;

	void SetCamera(FreeCamera* cam) { m_pCamera = cam; }

private:
	FreeCamera* m_pCamera = nullptr;
};

