#pragma once
#include "Component/IComponent/IComponent.h"
#include <string>
#include "system/BoneCombMatrix.h"

class CAnimationMesh;
class CAnimationData;
class aiAnimation;

class Animator : public IComponent
{
public:
    Animator(CAnimationMesh* mesh, CAnimationData* data, const std::string& defaultAnim);
    ~Animator();

    void Init(void) override;
    void Update(float dt);
	const BoneCombMatrix& GetBoneMatrix(void) const { return m_BoneMatrix; }

	void AdvanceFrame(float dt);

private:
    // アニメーション対象
    CAnimationMesh* m_TargetMesh = nullptr;
    CAnimationData* m_AnimData = nullptr;

    // 現在のアニメーション
    aiAnimation* m_CurrentAnimation = nullptr;
    std::string  m_DefaultAnimation;

    // ボーン行列計算結果
    BoneCombMatrix m_BoneMatrix;

    // 再生制御
    float m_CurrentFrame = 0.0f;
    float m_PlaySpeed = 1.0f;
};