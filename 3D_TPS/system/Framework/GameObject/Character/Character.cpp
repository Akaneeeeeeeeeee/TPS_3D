#include "Character.h"
#include "system/Framework/AssetManager/AssetManager.h"

void Character::Init(void)
{
	// キャラクターの初期化処理
	// 例: モデルの読み込み、アニメーションの設定など
}

void Character::Update(uint64_t deltatime)
{
	// キャラクターの更新処理
	// 例: 入力に基づく移動、アニメーションの更新など
	m_pAnimationObject->Update(m_AnimationSpeed);
}

void Character::Draw(const uint64_t deltatime) const
{
	// キャラクターの描画処理
	// 例: モデルの描画、エフェクトの適用など
	
	// シェーダーをセット
	m_Shader.SetGPU();

	// ワールド行列をセット
	Matrix4x4 worldMatrix = this->GetWorldMatrix();
	Renderer::SetWorldMatrix(&worldMatrix);

	m_pAnimationObject->Draw();
}

void Character::Uninit(void)
{
	// キャラクターの終了処理
	// 例: リソースの解放など
}