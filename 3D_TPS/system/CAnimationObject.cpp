#include	"CAnimationObject.h"

using namespace DirectX::SimpleMath;

void CAnimationObject::Init()
{
	// ボーンコンビネーション行列初期化
	m_BoneCombMatrix.Create();							// 20240723 
}

void CAnimationObject::Update(float dt) 
{
	int frame = static_cast<int>(m_CurrentFrame);
	// アニメーションメッシュ更新
	m_AnimMesh->Update(m_BoneCombMatrix,frame);
	m_CurrentFrame+=dt;
}

void CAnimationObject::Draw()
{
	// ボーンコンビネーション行列用定数バッファ更新
	m_BoneCombMatrix.Update();

	// 定数バッファGPUへセット
	m_BoneCombMatrix.SetGPU();

	// メッシュ描画
	m_AnimMesh->Draw();
}

