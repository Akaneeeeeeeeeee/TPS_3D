#pragma once
#include "system/Framework/GameObject/GameObject.h"

struct EngineContext;

class Rock final : public GameObject
{
public:
	Rock();
	Rock(EngineContext& context, uint64_t id, const std::string& name = "", const Tag& tag = Tag::None);
	~Rock();

	void Init(void) override;
	void Update(uint64_t) override;
	void Draw(uint64_t) override;
	void Uninit(void) override;

private:

};

