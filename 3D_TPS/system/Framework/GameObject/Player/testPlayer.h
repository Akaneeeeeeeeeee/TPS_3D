#pragma once
#include "GameObject/Character/Character.h"

class TestPlayer : public Character
{
public:
	TestPlayer(EngineContext& context, const uint64_t id,
		const std::string& name = "", const Tag& tag = Tag::None,
		const Transform& transform = Transform::One())
		: Character(context, id, name, tag, transform)
	{
	};
	~TestPlayer() = default;
	void Init(void) override;
	void Update(const uint64_t deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

private:

};

