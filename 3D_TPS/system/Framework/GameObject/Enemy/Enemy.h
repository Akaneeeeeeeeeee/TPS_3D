#pragma once
#include "system/Framework/GameObject/Character/Character.h"


class Enemy : public Character
{
public:
	Enemy() = default;
	Enemy(uint64_t id, const std::string& name = "", const Tag& tag = Tag::Player);
	~Enemy();

	void Init(void) override;
	void Update(uint64_t deltatime) override;
	void Draw(uint64_t deltatime) override;
	void Uninit(void) override;

private:
	// ‹–ì
	float m_ViewAngle = 60.0f;
	// Å‘å‹–ì‹——£
	float m_ViewDistance = 20.0f;
};

