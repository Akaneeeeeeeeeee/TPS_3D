#pragma once
#include "system/CAnimationData.h"
#include "system/CAnimationMesh.h"
#include "system/CAnimationObject.h"
#include "system/Framework/GameObject/GameObject.h"


class Character : public GameObject
{
public:
	Character();
	Character(uint64_t id, const std::string& name = "", const Tag& tag = Tag::None)
		: GameObject(id, name, tag)
	{
	};
	virtual ~Character() {};

	void Init(void);
	void Update(uint64_t deltatime);
	void Draw(uint64_t deltatime);
	void Uninit(void);

	void SetAnimationData(CAnimationData* pAnimationData) { m_pAnimationData = pAnimationData; }
	void SetAnimationMesh(CAnimationMesh* pAnimationMesh) { m_pAnimationMesh = pAnimationMesh; }

private:
	CAnimationData* m_pAnimationData;
	CAnimationMesh* m_pAnimationMesh;
	std::unique_ptr<CAnimationObject> m_pAnimationObject;
};
