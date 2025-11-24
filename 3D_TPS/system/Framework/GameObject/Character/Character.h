#pragma once
#include "system/CAnimationData.h"
#include "system/CAnimationMesh.h"
#include "system/CAnimationObject.h"
#include "system/CShader.h"
#include "system/Framework/GameObject/GameObject.h"

/// <summary>
/// アニメーションの状態を表す列挙型
/// </summary>
enum class AnimationState {
	Idle,
	Run,
	Jump,
	Shot,
	Damage,
	Death
};

class SkinnedAnimationComponent;

/// <summary>
/// アニメーション付きキャラクターオブジェクト
/// </summary>
class Character : public GameObject
{
public:
	Character() = delete;
	Character(ComponentFactory* factory, const uint64_t id, 
		const std::string& name = "", const Tag& tag = Tag::None,
		const Transform& transform = Transform::One())
		: GameObject(factory, id, name, tag, transform)
	{
	};
	virtual ~Character() {};

	virtual void Init(void);
	virtual void Update(const float deltatime) override;
	virtual void Draw(void) const override;
	virtual void Uninit(void);

	virtual void SetAnimationData(CAnimationData* pAnimationData) { m_pAnimationData = pAnimationData; }
	virtual void SetAnimationMesh(CAnimationMesh* pAnimationMesh) { m_pAnimationMesh = pAnimationMesh; }

protected:
	CAnimationData* m_pAnimationData;
	CAnimationMesh* m_pAnimationMesh;
	std::unique_ptr<CAnimationObject> m_pAnimationObject;
	CShader m_Shader;

	aiAnimation* m_pCurrentAnimation = nullptr;
	AnimationState m_AnimState = AnimationState::Idle;

	SkinnedAnimationComponent* m_pAnimComp = nullptr;

	float m_AnimationSpeed = 1.0f;
	float m_MoveSpeed = 7.5f;
};
