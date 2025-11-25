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

	void SetCamera(FreeCamera* cam) { m_pCamera = cam; }

private:
	FreeCamera* m_pCamera = nullptr;
	CharacterVirtualComponent* m_pCharaVirtualComp = nullptr;
};

