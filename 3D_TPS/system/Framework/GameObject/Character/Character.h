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
class CharacterVirtualComponent;
class SoundEmitterComponent;

/// <summary>
/// アニメーション付きキャラクターオブジェクト
/// </summary>
class Character : public GameObject
{
public:
	DECLARE_GAMEOBJECT_TYPE(Character, GameObject);
	Character() = delete;
	Character(ComponentFactory* factory, const uint64_t id, 
		const std::string& name = "", const Tag& tag = Tag::None,
		const Transform& transform = Transform::One())
		: GameObject(factory, id, name, tag, transform)
	{
	};
	virtual ~Character() {};

	virtual void Awake(void);
	virtual void Update(const float deltatime) override;
	virtual void Draw(void) const override;
	virtual void Uninit(void);


protected:
	aiAnimation* m_pCurrentAnimation = nullptr;
	AnimationState m_AnimState = AnimationState::Idle;

	SkinnedAnimationComponent* m_pAnimComp = nullptr;
	CharacterVirtualComponent* m_pCharVirtual = nullptr;
	SoundEmitterComponent* m_pSoundEmitter = nullptr;

	struct FootstepState {
		float timer = 0.0f;
		bool  wasOnGround = false;
		bool  isMoving = false;
	} m_Footstep;

	float m_AnimationSpeed = 1.0f;	// アニメーション速度倍率
	float m_MoveSpeed = 7.5f;		// 移動速度
};
