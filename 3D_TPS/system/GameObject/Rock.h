#pragma once
#include "system/Framework/GameObject/GameObject.h"

struct EngineContext;

class Rock final : public GameObject
{
public:
	//Rock();
	Rock(EngineContext& context, const uint64_t id, 
		const std::string& name = "", const Tag& tag = Tag::None, 
		const Transform& transform = Transform::One());
	~Rock();

	void Init(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

private:

};

