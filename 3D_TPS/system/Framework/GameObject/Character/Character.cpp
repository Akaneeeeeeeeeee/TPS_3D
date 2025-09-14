#include "Character.h"
#include "system/Framework/AssetManager/AssetManager.h"

void Character::Init(void)
{
	// キャラクターの初期化処理
	// 例: モデルの読み込み、アニメーションの設定など
	
	
	// アニメーションオブジェクトを生成
	this->m_pAnimationObject = std::make_unique<CAnimationObject>();
	this->m_pAnimationObject->Init();

	// メッシュを取得
	this->m_pAnimationMesh = AssetManager::GetInstance().GetAnimationMesh("Akai");
	// シェーダーの初期化
	m_Shader.Create("shader/vertexLightingOneSkinVS.hlsl", "shader/vertexLightingPS.hlsl");
	// アニメーションデータ取得
	this->m_pAnimationData = AssetManager::GetInstance().GetAnimationData("Akai_Idle");
	// 現在のアニメーションをセット
	aiAnimation* animation = m_pAnimationData->GetAnimation("Akai_Idle", 0);
	this->m_pCurrentAnimation = animation;
	m_pAnimationMesh->SetCurentAnimation(animation);
	// アニメーションメッシュをセット
	this->m_pAnimationObject->SetAnimationMesh(m_pAnimationMesh);
}

void Character::Update(uint64_t deltatime)
{
	// キャラクターの更新処理
	// 例: 入力に基づく移動、アニメーションの更新など
	m_pAnimationObject->Update(m_AnimationSpeed);
}

void Character::Draw(uint64_t deltatime)
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