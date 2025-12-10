#pragma once
#include "Framework/GameObject/GameObject.h"

class ParticleComponent;

class WeatherController : public GameObject
{
public:
	WeatherController() = default;
	WeatherController(ComponentFactory* factory, const uint64_t id,
		const std::string& name = "", const Tag& tag = Tag::Player,
		const Transform& transform = Transform::One());
	~WeatherController();

	void Init(void) override;
	void Update(const float deltatime) override;
	//void Draw(void) const override;
	void Uninit(void) override;

private:
	ParticleComponent* m_ParticleComp = nullptr;
};