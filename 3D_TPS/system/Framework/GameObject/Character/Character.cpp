#include "Character.h"
#include "system/Framework/AssetManager/AssetManager.h"

void Character::Init(void)
{
	// キャラクターの初期化処理
	// 例: モデルの読み込み、アニメーションの設定など

	// メッシュを取得
	this->m_pAnimationMesh = AssetManager::GetInstance().GetAnimationMesh("Akai");

	// アニメーションオブジェクトを生成
	this->m_pAnimationObject = std::make_unique<CAnimationObject>();
	this->m_pAnimationObject->Init();

	// アニメーションメッシュをセット
	this->m_pAnimationObject->SetAnimationMesh(m_pAnimationMesh);

	// アニメーションデータ取得
	this->m_pAnimationData = AssetManager::GetInstance().GetAnimationData("Akai_Idle");

	// 現在のアニメーションをセット
	aiAnimation* animation = m_pAnimationData->GetAnimation("Akai_Idle", 0);
	m_pAnimationMesh->SetCurentAnimation(animation);
}

void Character::Update(uint64_t deltatime)
{
	// キャラクターの更新処理
	// 例: 入力に基づく移動、アニメーションの更新など
	m_pAnimationObject->Update(1.0f);
}

void Character::Draw(uint64_t deltatime)
{
	// キャラクターの描画処理
	// 例: モデルの描画、エフェクトの適用など
	m_pAnimationObject->Draw();
}

void Character::Uninit(void)
{
	// キャラクターの終了処理
	// 例: リソースの解放など
}